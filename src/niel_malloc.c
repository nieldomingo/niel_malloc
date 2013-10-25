#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "niel_malloc.h"


mem_blk_header  *allocate_new_blk(mem_blk_header *last_blk, size_t size)
{
    size_t size_to_allocate = mem_blk_header_size + size;

    mem_blk_header *new_blk = sbrk(size_to_allocate);

    if (new_blk == (mem_blk_header *)-1)
        return NULL;
    
    new_blk->mem_blk = calc_mem_blk(new_blk);
    new_blk->blk_size = size;
    new_blk->next = NULL;
    new_blk->prev = last_blk;
    new_blk->assigned_flag = 1;
    if (last_blk != NULL)
    {
        last_blk->next = new_blk;
    }

    return new_blk->mem_blk;
}

mem_blk_header *assign_blk(mem_blk_header *blk, size_t size)
{
    size_t size_diff = blk->blk_size - size - mem_blk_header_size;

    if (size_diff > mem_blk_header_size)
    {
        /*
         * split the block
         */

        blk->blk_size = size;
        blk->assigned_flag = 1;
        mem_blk_header *new_blk = (mem_blk_header *)((char *)blk + mem_blk_header_size + size);
        new_blk->mem_blk = calc_mem_blk(new_blk);
        new_blk->blk_size = size_diff;
        new_blk->assigned_flag = 0;
        new_blk->next = blk->next;
        new_blk->prev = blk;
        blk->next = new_blk;

        return blk->mem_blk;
    }
    else
    {
        /*
         * don't split the block
         */
        blk->assigned_flag = 1;

        return blk->mem_blk;
    }
}

void *niel_malloc(size_t size)
{
    size_t aligned_size = align8(size);

    /* first call */
    if (heap_start == NULL)
    {
        heap_start = sbrk(0);
        return allocate_new_blk(NULL, aligned_size);
    }
    else
    {
        mem_blk_header *prev = heap_start;
        mem_blk_header *current = heap_start;

        while(current != NULL)
        {
            /*
             * look for a suitable niel_free block.
             * very naive algorithm, just assign the block
             * that fits.
             */
            if (current->assigned_flag == 0 && current->blk_size >= aligned_size)
            {
                return assign_blk(current, aligned_size);
            }

            prev = current;
            current = current->next;
        }

        /* 
         * did not find a suitable block, will allocate
         * new block
         * */

        return allocate_new_blk(prev, aligned_size);
    }
}

void collapse_consecutive_blks(mem_blk_header *start_blk)
{
    mem_blk_header *next_blk = start_blk->next;
    size_t new_size = start_blk->blk_size + next_blk->blk_size + mem_blk_header_size;

    start_blk->blk_size = new_size;
    start_blk->next = next_blk->next;

    if (next_blk->next != NULL)
    {
        next_blk->next->prev = start_blk;
    }
}

void niel_free(void *ptr)
{
    mem_blk_header *blk = calc_head_blk(ptr);
    
    blk->assigned_flag = 0;

    /*
     * check if current blk is at the end
     */
    if (blk->next == NULL)
    {
        if (blk->prev == NULL)
        {
            sbrk(-total_blk_size(blk));
            heap_start = NULL;
        }
        else if (blk->prev != NULL && blk->prev->assigned_flag == 1)
        {
            mem_blk_header *prev = blk->prev;
            prev->next = NULL;
            sbrk(-total_blk_size(blk));
        }
        else if (blk->prev != NULL && blk->prev->assigned_flag == 0)
        {
            mem_blk_header *prev_prev = blk->prev->prev;
            mem_blk_header *prev = blk->prev;
            if (prev_prev != NULL)
            {
                prev_prev->next = NULL;
            }
            else
            {
                heap_start = NULL;
            }
            sbrk(-total_blk_size(blk) - total_blk_size(prev));
        }

        return;
    }

    /*
     * check if succeeding blk can be combined with current block
     */
    if (blk->next != NULL && blk->next->assigned_flag == 0)
    {
        collapse_consecutive_blks(blk);
    }

    /*
     * check if preceeding blk can be combined with current block
     */
    if (blk->prev != NULL && blk->prev->assigned_flag == 0)
    {
        blk = blk->prev;
        collapse_consecutive_blks(blk);
    }

}

void *niel_calloc(size_t nr, size_t size)
{
    void *mem_ptr = NULL;
    
    mem_ptr = niel_malloc(nr * size);

    memset(mem_ptr, '\0', nr * size);

    return mem_ptr;
}

void *niel_realloc(void *ptr, size_t size)
{
    mem_blk_header *blk = calc_head_blk(ptr);
    size_t size_to_allocate = align8(size);

    if (blk->next == NULL)
    {
        /* case: the blk is a the end of the heap and we
         * can fulfill the request by allocating more memory
         */


        void *new_mem = sbrk(size_to_allocate);
        if (new_mem == (void *)-1)
            return NULL;

        blk->blk_size += size_to_allocate;

        return ptr;
    }
    else
    {
        if (blk->next->assigned_flag == 0 && \
            total_blk_size(blk->next) >= size_to_allocate)
        {
            /*
             * case: the current block precedes a free block that can be allocated
             * to satisfy the requested memory.
             */
            size_t excess_size = total_blk_size(blk->next) - size_to_allocate;
            if (excess_size >= MINIMUM_BLK_SIZE) 
            {
                mem_blk_header *new_blk = (mem_blk_header *)((char *)(blk->mem_blk) + size_to_allocate);
                new_blk->prev = blk;
                new_blk->next = blk->next;
                new_blk->blk_size = excess_size - mem_blk_header_size;
            }
            else
            {
                blk->next = blk->next->next;
                blk->blk_size = total_blk_size(blk->next);
            }
        }
        else if (blk->prev->assigned_flag == 0 && \
            total_blk_size(blk->prev) >= size_to_allocate && \
            blk->next->assigned_flag == 1)
        {
            /*
             * case: current block follows a free block that can be allocated
             * to satisfy the requested memory.
             */
            mem_blk_header *prev = blk->prev;
            
            size_t excess_size = total_blk_size(prev) - size_to_allocate;
            if (excess_size >= MINIMUM_BLK_SIZE)
            {
                mem_blk_header *next = blk->next;
                memmove(prev->mem_blk, blk->mem_blk, blk->blk_size);

                mem_blk_header *new_blk = (mem_blk_header *)((char *)(prev->mem_blk) + \
                        prev->blk_size + size_to_allocate);
                new_blk->assigned_flag = 0;
                new_blk->prev = prev;
                new_blk->next = next;
                new_blk->mem_blk = calc_mem_blk(new_blk);
                new_blk->blk_size = prev->blk_size - size_to_allocate;
                
                prev->assigned_flag = 1;
                prev->blk_size += size_to_allocate;
                prev->next = new_blk;

            }
            else
            {
                prev->assigned_flag = 1;
                prev->blk_size += total_blk_size(blk);
                prev->next = blk->next;
                
                memmove(prev->mem_blk, blk->mem_blk, blk->blk_size);
            }
        }
        else if (blk->prev->assigned_flag == 0 && blk->next->assigned_flag == 0 && \
            (total_blk_size(blk->prev) + total_blk_size(blk->next)) >= size_to_allocate )
        {
            size_t excess_size = total_blk_size(blk->prev) + total_blk_size(blk->next) - size_to_allocate;
            if (excess_size >= MINIMUM_BLK_SIZE)
            {
                /*
                 * save addresses and sizes
                 */
                mem_blk_header *prev = blk->prev;
                mem_blk_header *next_next = blk->next->next;
                size_t current_size = blk->blk_size;

                memmove(prev->mem_blk, blk->mem_blk, blk->blk_size);

                mem_blk_header *new_blk = (mem_blk_header *)((char *)(prev->next) + size_to_allocate);
                new_blk->assigned_flag = 0;
                new_blk->prev = prev;
                new_blk->next = next_next;
                new_blk->mem_blk = calc_mem_blk(new_blk);
                new_blk->blk_size = excess_size - mem_blk_header_size;

                prev->assigned_flag = 1;
                prev->blk_size = current_size + size_to_allocate;
                prev->next = new_blk;

            }
            else
            {
                mem_blk_header *prev = blk->prev;

                prev->assigned_flag = 1;
                prev->blk_size = blk->blk_size + size_to_allocate;
                prev->next = blk->next->next;

                memmove(prev->mem_blk, blk->mem_blk, blk->blk_size);
            }
        }
    }
}

/*
 * TODO: write tests for each allocation function
 */

/*
int main(int argc, char *argv[])
{
    int *arr1, *arr2, *arr3;

    arr1 = (int *)niel_malloc(sizeof(int) * 100);
    arr2 = (int *)niel_malloc(sizeof(int) * 100);
    arr3 = (int *)niel_malloc(sizeof(int) * 100);

    niel_free(arr1);
    niel_free(arr2);
    niel_free(arr3);

    arr1 = (int *)niel_malloc(sizeof(int) * 150);
    arr2 = (int *)niel_malloc(sizeof(int) * 150);

    niel_free(arr1);
    niel_free(arr2);

    arr1 = (int *)niel_calloc(100, sizeof(int));
    arr2 = (int *)niel_calloc(100, sizeof(int));
    arr3 = (int *)niel_calloc(100, sizeof(int));

    niel_free(arr1);
    niel_free(arr2);
    niel_free(arr3);

}
*/
