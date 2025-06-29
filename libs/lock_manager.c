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

#include "lock_manager.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

size_t lock_hash ( uint64_t row_id ) {
    return row_id % LOCK_TABLE_SIZE;
}

void init_lock_manager ( lock_manager_t * mgr ) {
    //
    memset ( mgr , 0 , sizeof ( lock_manager_t ) );
    //
    for ( int i = 0 ; i < LOCK_TABLE_SIZE ; i++ ) {
        //
        memset ( & mgr->buckets[ i ] , 0 , sizeof ( lock_bucket_t ) );
        //
        pthread_rwlock_init ( & mgr->buckets[ i ].rwlock , NULL );
        //
        for ( int j = 0 ; j < LOCK_TABLE_SIZE ; j++ ) {
            //
            memset ( & mgr->buckets[ i ].entries[ j ] , 0 , sizeof ( row_lock_t ) );
            //
            mgr->buckets[ i ].entries[ j ].is_used = false;
            //
        }
    }
}

lock_manager_t * create_lock_manager ( ) {
    lock_manager_t * mgr = malloc ( sizeof ( lock_manager_t ) );
    init_lock_manager ( mgr );
    return mgr;
}

void free_lock_manager ( lock_manager_t * mgr ) {
    if ( mgr != NULL ) {
        free ( mgr );
    }
}

static row_lock_t * probe_bucket ( lock_bucket_t * bucket , uint64_t row_id ) {
    size_t    index = lock_hash ( row_id );
    for ( int i     = 0 ; i < MAX_PROBE_DEPTH ; i++ ) {
        size_t probe_idx = ( index + i ) % LOCK_TABLE_SIZE;
        if ( !bucket->entries[ probe_idx ].is_used ||
             bucket->entries[ probe_idx ].row_id == row_id ) {
            return & bucket->entries[ probe_idx ];
        }
    }
    return NULL;
}

static row_lock_t * find_or_create_lock ( lock_bucket_t * bucket , uint64_t row_id ) {
    pthread_rwlock_wrlock ( & bucket->rwlock );

    row_lock_t * slot = probe_bucket ( bucket , row_id );
    if ( !slot ) {
        pthread_rwlock_unlock ( & bucket->rwlock );
        return NULL;
    }

    if ( !slot->is_used ) {
        pthread_mutex_init ( & slot->mutex , NULL );
        pthread_cond_init ( & slot->cond , NULL );
        slot->row_id   = row_id;
        slot->refcount = 0;
        slot->is_used  = true;
    }

    pthread_rwlock_unlock ( & bucket->rwlock );
    return slot;
}

int acquire_lock ( lock_manager_t * mgr , uint64_t row_id ) {
    lock_bucket_t * bucket = & mgr->buckets[ lock_hash ( row_id ) ];
    row_lock_t    * lock   = find_or_create_lock ( bucket , row_id );
    if ( !lock ) return ENOMEM;

    pthread_mutex_lock ( & lock->mutex );
    if ( lock->refcount == REFCOUNT_MAX ) {
        pthread_mutex_unlock ( & lock->mutex );
        return EOVERFLOW;
    }
    lock->refcount++;
    pthread_mutex_unlock ( & lock->mutex );
    return 0;
}

void release_lock ( lock_manager_t * mgr , uint64_t row_id ) {
    lock_bucket_t * bucket = & mgr->buckets[ lock_hash ( row_id ) ];
    pthread_rwlock_rdlock ( & bucket->rwlock );

    row_lock_t * lock = probe_bucket ( bucket , row_id );
    if ( !lock || !lock->is_used || lock->row_id != row_id ) {
        pthread_rwlock_unlock ( & bucket->rwlock );
        return;
    }

    pthread_mutex_lock ( & lock->mutex );
    if ( lock->refcount > 0 ) {
        lock->refcount--;
    }
    pthread_mutex_unlock ( & lock->mutex );
    pthread_rwlock_unlock ( & bucket->rwlock );
}
