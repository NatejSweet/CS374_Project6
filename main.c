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
        struct block *next_block = PTR_OFFSET(head, PADDED_SIZE(sizeof(struct block)) + actual_size);
        head->size = data_size;
        next_block->size = head->size - actual_size - PADDED_SIZE(sizeof(struct block));
        head->in_use = 0;
        head->next = next_block;
        first_call = 0;
    }
    if (head->in_use == 0 && head->size >= actual_size)
    {
        head->in_use = 1;
        head->size = actual_size;
        return (int *)PTR_OFFSET(head, sizeof(struct block));
    }
    else
    {
        struct block *current = head;
        while (current->next != NULL && (current->in_use == 1 || current->size < actual_size))
        {
            current = current->next;
        }
        if (current->size < actual_size)//ensure there is room(may need to add size of another node, not sure, works for now)
        {
            printf("No space available\n");
            return NULL;
        }
        else{
            if (current->next == NULL)
            {
                struct block *next_block = PTR_OFFSET(current, PADDED_SIZE(sizeof(struct block)) + actual_size); // next_block is the new block
                next_block->size = current->size - actual_size - PADDED_SIZE(sizeof(struct block)); // size of the new block = size of the current block - size of the requested block - size of the block struct
                current->in_use = 1;
                current->size = actual_size; // size of the current block = size of the requested block
                current->next = next_block;  // next of the current block = next_block
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
{   ptr = (int *)PTR_OFFSET(ptr, -sizeof(struct block));
    struct block *current = head;
    while (current != NULL)
    {
        if (current == ptr)
        {
            current->in_use = 0;
            return 0;
        }
        current = current->next;
    }
    return -1;
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
void *p;

myalloc(10);     print_data();
p = myalloc(20); print_data();
myalloc(30);     print_data();
myfree(p);       print_data();
myalloc(40);     print_data();
myalloc(10);     print_data();
}