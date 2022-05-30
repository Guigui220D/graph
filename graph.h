#ifndef INO_GRAPH_H
#define INO_GRAPH_H

#include <stddef.h>
#include <pthread.h>
#include <stdatomic.h>

typedef struct {
    atomic_int must_stop;
    pthread_t thread;
    float* data;
    size_t size;
} GraphContext;

int inog_init_resources();
void inog_deinit_resources();

GraphContext* inog_make_graph(const float * data, size_t size);
void inog_destroy_graph(GraphContext* context);

#endif