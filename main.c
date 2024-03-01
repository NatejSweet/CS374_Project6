#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define ALIGNMENT 16 // Must be power of 2
#define GET_PAD(x) ((ALIGNMENT - 1) - (((x)-1) & (ALIGNMENT - 1)))
#define PADDED_SIZE(x) ((x) + GET_PAD(x))
#define PTR_OFFSET(p, offset) ((void *)((char *)(p) + (offset)))

struct block
{
    int size;
    int in_use;
    struct block *next;
};
void *heap;
struct block *head = NULL;

void *myalloc(int size)
{
    static int first_call = 1;
    int data_size = 1024 - PADDED_SIZE(sizeof(struct block));
    int actual_size = PADDED_SIZE(size);
    if (first_call)
    {
        heap = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
        head = (struct block *)heap;
        head->size = data_size;
        head->in_use = 0;
        head->next = NULL;
        first_call = 0;
    }
    if (head->in_use == 0 && head->size >= (int)(actual_size+PADDED_SIZE(sizeof(struct block))))       //if the first block is free and has enough space
    {
        struct block *next_block = PTR_OFFSET(head, PADDED_SIZE(sizeof(struct block)) + actual_size);   
        next_block->size = head->size - actual_size - PADDED_SIZE(sizeof(struct block));
        head->next = next_block;
        head->in_use = 1;
        head->size = actual_size;
        return (int *)PTR_OFFSET(head, sizeof(struct block));
    }
    else
    {
        struct block *current = head;
        while (current->next != NULL && (current->in_use == 1 || current->size < actual_size))  //if the first block is not free or does not have enough space
        {
            current = current->next;
        }
        if (current->size < actual_size || current->in_use == 1)//if no space is available
        {
            printf("No space available\n"); //another heap could be allocated here to allow limitless allocation(beyond 1024 bytes)
            return NULL;
        }
        else{
            if (current->next == NULL && current->size >= (int)(actual_size+PADDED_SIZE(sizeof(struct block))))  //if is the last block and has enough space
            {
                struct block *next_block = PTR_OFFSET(current, PADDED_SIZE(sizeof(struct block)) + actual_size);
                next_block->size = current->size - actual_size - PADDED_SIZE(sizeof(struct block)); 
                current->in_use = 1;
                current->size = actual_size;
                current->next = next_block; 
                return (int *)PTR_OFFSET(current, sizeof(struct block));    
            }
            else
            {
                current->in_use = 1;
                return (int *)PTR_OFFSET(current, sizeof(struct block));
            }
        }
    }
}

int myfree(int *ptr)
{
    struct block *current = head;
    while (current != NULL)
    {
        if ((int *)PTR_OFFSET(current, sizeof(struct block)) == ptr)//if current is the target
        {
            current->in_use = 0;
            return coalesceFrom(head);
        }
        current = current->next;
    }
    return -1;
}

int coalesceFrom(struct block *start)
{
    struct block *current = start;
    while (current->next != NULL)
    {
        if (current->in_use == 0 && current->next->in_use == 0) //if both current and next are free
        {
            current->size += current->next->size + PADDED_SIZE(sizeof(struct block));
            current->next = current->next->next;
            return coalesceFrom(current);   //recursively coalesce, from the same node in case of multiple free blocks in a row. 
        }
        current = current->next;
    }
    return 0;
}


void print_data(void)
{
    struct block *b = head;

    if (b == NULL)
    {
        printf("[empty]\n");
        return;
    }

    while (b != NULL)
    {
        // Uncomment the following line if you want to see the pointer values
        // printf("[%p:%d,%s]", b, b->size, b->in_use? "used": "free");
        printf("[%d,%s]", b->size, b->in_use ? "used" : "free");
        if (b->next != NULL)
        {
            printf(" -> ");
        }

        b = b->next;
    }

    printf("\n");
}

int main(int argc, char *argv[])
{
void *p, *q, *r, *s;

p = myalloc(10); print_data();
q = myalloc(20); print_data();
r = myalloc(30); print_data();
s = myalloc(40); print_data();

myfree(q); print_data();
myfree(p); print_data();
myfree(s); print_data();
myfree(r); print_data();
}