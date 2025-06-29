#ifndef LOCK_MANAGER_H
#define LOCK_MANAGER_H

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

#define LOCK_TABLE_SIZE 1024
#define MAX_PROBE_DEPTH 8
#define REFCOUNT_MAX UINT64_MAX

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    uint64_t        refcount;
    uint64_t        row_id;
    bool is_used;
} row_lock_t;

typedef struct {
    pthread_rwlock_t rwlock;
    row_lock_t       entries[LOCK_TABLE_SIZE];
} lock_bucket_t;

typedef struct {
    lock_bucket_t buckets[LOCK_TABLE_SIZE];
} lock_manager_t;

lock_manager_t * create_lock_manager ( ) ;

void free_lock_manager ( lock_manager_t * mgr ) ;

int acquire_lock ( lock_manager_t * mgr , uint64_t row_id );

void release_lock ( lock_manager_t * mgr , uint64_t row_id );

#endif
