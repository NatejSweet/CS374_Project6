#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define ALIGNMENT 16   // Must be power of 2
#define GET_PAD(x) ((ALIGNMENT - 1) - (((x) - 1) & (ALIGNMENT - 1)))
#define PADDED_SIZE(x) ((x) + GET_PAD(x))
#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))

struct block {
    int size;
    int in_use;
    struct block *next;
};
void *heap;
struct block *head = NULL;



int *myalloc(int size) {
    static int first_call = 1;
    int data_size = 1024 - PADDED_SIZE(sizeof(struct block));
    int actual_size = PADDED_SIZE(size);
    if (first_call) {
        heap = mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
        head = (struct block *)heap;
        head->size = data_size;
        head->in_use = 0;
        head->next = NULL;
        first_call = 0;
    }
    if (head->in_use == 0 && head->size >= actual_size) {
        head->in_use = 1;
        return (int *)PTR_OFFSET(head, sizeof(struct block));
    }else {
        struct block *current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        return NULL;
        // current->next = PTR_OFFSET(current, PADDED_SIZE(sizeof(struct block)) + actual_size);
        // current->next->size = data_size;
        // current->next->in_use = 0;
        // current->next->next = NULL;

    }
}
    
int myfree(int *ptr) {
    return 0;
}

void print_data(void)
{
    struct block *b = head;

    if (b == NULL) {
        printf("[empty]\n");
        return;
    }

    while (b != NULL) {
        // Uncomment the following line if you want to see the pointer values
        //printf("[%p:%d,%s]", b, b->size, b->in_use? "used": "free");
        printf("[%d,%s]", b->size, b->in_use? "used": "free");
        if (b->next != NULL) {
            printf(" -> ");
        }

        b = b->next;
    }

    printf("\n");
}

int main(int argc, char *argv[]) {
        void *p;

    print_data();
    p = myalloc(16);
    print_data();
    p = myalloc(16);
    printf("%p\n", p);
}