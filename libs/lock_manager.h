/**
* Created by Clion.
* User: huc0day
* Date: 1-1-20
* Time: 上午11:36
*/
﻿
/*
=======================================================================================================
NoSQL_Storage_Engine is a high-performance memory based NOSQL database engine.
Copyright (C) 2020-2025, huc0day (Chinese name: GaoJian).
All rights reserved.
﻿
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
﻿
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY;   without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
GNU General Public License for more details.
﻿
You should have received a copy of the GNU General Public License
along with this program.    If not, see <https://www.gnu.org/licenses/>.
=======================================================================================================
*/

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
