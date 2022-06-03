#ifndef INOG_HEADER_GUARD
#define INOG_HEADER_GUARD

int inog_init_resources(void);
void inog_deinit_resources(void);

void* inog_make_context(const float *, unsigned int);
void inog_destroy_context(void*);
void inog_wait_destroy_context(void*);

#endif