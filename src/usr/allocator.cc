#include "common/stdlib.hh"

/*
    Simple userland switcher to allow extensibly building userland programs
*/
int process_main(int cmd) {
    switch (cmd) {
        case 0: {
            printf("Hello, 0!\n");
            break;
        }
        case 1: {
            printf("Hello, 1!\n");
            break;
        }
        default: {
            printf("Hello, userland!\n");
        }
    }

    printf("My pid is %i!\n", sys_getpid());

    int* pg;
    // allocate until out of memory
    while (1) {
        pg = (int*) sys_page_alloc();
        if (pg != nullptr) {
            *pg = 42;
            assert(*pg == 42);
            sys_page_free(pg);
        } else {
            printf("out of mem\n");
            break;
        }

        sys_yield();
    }

    // yield forever
    while (1) {
        sys_yield();
    }

    return 1;
}