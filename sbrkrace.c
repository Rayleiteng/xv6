#include "types.h"
#include "user.h"

int fail = 0;

void work(void *arg1, void *arg2) {
    int id = (int)arg1;
    int i, j;
    char *p;

    for(i = 0; i < 100; i++){
        p = malloc(100);
        if(p == 0) {
            printf(1, "malloc null\n");
            break;
        }

        for(j = 0; j < 100; j++) 
            p[j] = id;
        
        sleep(1);

        for(j = 0; j < 100; j++){
            if(p[j] != id){
                printf(1, "race detected! expected %d got %d\n", id, p[j]);
                fail = 1;
                exit();
            }
        }
    }
    exit();
}

int main(void) {
    int i;
    
    printf(1, "test sbrk race...\n");

    for(i = 1; i <= 4; i++)
        thread_create(work, (void*)i, 0);

    for(i = 1; i <= 4; i++)
        thread_join();

    if(fail)
        printf(1, "FAIL\n");
    else
        printf(1, "PASS\n");
        
    exit();
}