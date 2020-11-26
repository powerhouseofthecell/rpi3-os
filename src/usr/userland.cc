#include "usr/userland.hh"
#include "common/stdlib.hh"

/*
    Simple userland switcher to allow extensibly building userland programs
*/
int user_main(int cmd) {
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

    while (1) {
        sys_yield();
    }

    return 1;
}