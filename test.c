#include <stdio.h>
#include "src/header.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

int main(int argc, char* argv[]) 
{
    printf("Yooo\n");
    if (inog_init_resources()) {
        printf("Inog failed to init resources\n");
        return -1;
    }

    void* context = inog_make_context(NULL, 0);
    sleep(2);
    void* context2 = inog_make_context(NULL, 0);
    sleep(5);
    inog_destroy_context(context2);
    sleep(2);
    inog_destroy_context(context);

    inog_deinit_resources();
    return 0;
}