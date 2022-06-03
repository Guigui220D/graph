#include <stdio.h>
#include <math.h>
#include "src/header.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

void testFn1(float * buffer, size_t size) {
    for (int i = 0; i < size; i++) {
        float x = (float)i / 1000.f;
        x *= 10;
        //buffer[i] = cosf(x * x) * x * x;
        buffer[i] = (i / 50) % 3;
    }
}

void testFn2(float * buffer, size_t size) {
    for (int i = 0; i < size; i++) {
        float x = (float)i / 1000.f;
        x *= 10;
        buffer[i] = cosf(x * x) * x * x;
        //buffer[i] = (i / 50) % 3;
    }
}

int main(int argc, char* argv[]) 
{
    const size_t SIZE = 1000;
    float samples1[SIZE];
    float samples2[SIZE];

    testFn1(samples1, SIZE);
    testFn2(samples2, SIZE);

    if (inog_init_resources()) {
        printf("Inog failed to init resources\n");
        return -1;
    }

    void* context = inog_make_context(samples1, SIZE);
    if (!context) {
        printf("Inog failed to make context\n");
        return -1;
    }
    sleep(2);
    void* context2 = inog_make_context(samples2, SIZE);
    if (!context2) {
        printf("Inog failed to make context\n");
        return -1;
    }
    sleep(5);
    inog_destroy_context(context2);
    sleep(2);
    inog_wait_destroy_context(context);

    inog_deinit_resources();
    return 0;
}