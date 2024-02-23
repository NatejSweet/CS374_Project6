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
        while (current->next != NULL && current->in_use == 1) // need to split later
        {
            current = current->next;
        }
        current = current->next;                                                                         // current->next == NULL
        struct block *next_block = PTR_OFFSET(current, PADDED_SIZE(sizeof(struct block)) + actual_size); // next_block is the new block
        printf("current size: %d\n", current->size);
        printf("next block exists? %d\n", next_block != NULL);
        next_block->size = current->size - actual_size - PADDED_SIZE(sizeof(struct block)); // size of the new block = size of the current block - size of the requested block - size of the block struct
        current->in_use = 1;
        current->size = actual_size; // size of the current block = size of the requested block
        current->next = next_block;  // next of the current block = next_block
        // printf("current size: %d\n", current->next->size);
        // next_block->size = current->next->size - actual_size - PADDED_SIZE(sizeof(struct block));
        // current->next->in_use = 1;
        // current->next->size = actual_size;
        // current->next->next = next_block;
        return (int *)PTR_OFFSET(current, sizeof(struct block));
    }
}

int myfree(int *ptr)
{
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
    void *p;
    void *p2;
    void *p3;

    print_data();
    p = myalloc(16);
    print_data();
    p2 = myalloc(16);
    print_data();
    p3 = myalloc(16);
    print_data();
    printf("%p\n", p);
}