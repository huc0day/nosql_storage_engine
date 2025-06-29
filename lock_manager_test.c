#include "libs/lock_manager.h"
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>

#define THREAD_NUM 16
#define TEST_ROUND 10000
#define THREADS 32
#define OPERATIONS 10000

static void * thread_exec ( void * arg ) {
    lock_manager_t * mgr = ( lock_manager_t * ) arg;
    for ( int i = 0 ; i < TEST_ROUND ; i++ ) {
        uint64_t row_id = 1000000000 + ( i % 10 );
        assert( acquire_lock ( mgr , row_id ) == 0 );
        // 模拟临界区操作
        release_lock ( mgr , row_id );
    }
    return NULL;
}

static void test_basic ( ) {
    //
    lock_manager_t * mgr =  create_lock_manager();
    // 测试基础锁定
    assert( acquire_lock ( mgr , 1000000000 ) == 0 );
    assert( acquire_lock ( mgr , 1000000001 ) == 0 );
    release_lock ( mgr , 1000000000 );
    release_lock ( mgr , 1000000001 );

    // 测试重复锁定
    for ( int i = 0 ; i < 10 ; i++ ) {
        assert( acquire_lock ( mgr , 1000000002 ) == 0 );
    }
    for ( int i = 0 ; i < 10 ; i++ ) {
        release_lock ( mgr , 1000000002 );
    }
    free(mgr);
}

static void test_concurrent ( ) {
    //
    lock_manager_t * mgr =  create_lock_manager();
    pthread_t threads[THREAD_NUM];

    for ( int i = 0 ; i < THREAD_NUM ; i++ ) {
        pthread_create ( & threads[ i ] , NULL , thread_exec , mgr );
    }
    for ( int i = 0 ; i < THREAD_NUM ; i++ ) {
        pthread_join ( threads[ i ] , NULL );
    }
    free(mgr);
}

static void test_edge_cases ( ) {
    lock_manager_t * mgr =  create_lock_manager();

    // 测试超大row_id
    uint64_t big_id = 10000000011000000001;
    assert( acquire_lock ( mgr , big_id ) == 0 );
    release_lock ( mgr , big_id );

    // 测试哈希冲突
    uint64_t id1 = 1000000000;  // 1000000000 % 1024 = 576
    uint64_t id2 = 1000001024;  // 1000001024 % 1024 = 576
    assert( acquire_lock ( mgr , id1 ) == 0 );
    assert( acquire_lock ( mgr , id2 ) == 0 );
    release_lock ( mgr , id1 );
    release_lock ( mgr , id2 );
    free(mgr);
}

lock_manager_t * lock_mgr;
int            shared_counter = 0;

void * worker ( void * arg ) {
    for ( int i = 0 ; i < OPERATIONS ; i++ ) {
        // 锁定要修改的行ID（示例使用固定ID）
        uint64_t row_id = 123456789;

        if ( acquire_lock ( lock_mgr , row_id ) != 0 ) {
            fprintf ( stderr , "Lock acquisition failed\n" );
            continue;
        }

        // 临界区开始
        shared_counter++;
        // 模拟业务处理耗时
        struct timespec ts = { 0 , 1000 };
        nanosleep ( & ts , NULL );
        // 临界区结束

        release_lock ( lock_mgr , row_id );
    }
    return NULL;
}

int pthread_lock_test ( ) {
    lock_mgr =  create_lock_manager();
    pthread_t threads[THREADS];

    // 创建线程池
    for ( int i = 0 ; i < THREADS ; i++ ) {
        pthread_create ( & threads[ i ] , NULL , worker , NULL );
    }

    // 等待所有线程完成
    for ( int i = 0 ; i < THREADS ; i++ ) {
        pthread_join ( threads[ i ] , NULL );
    }

    printf ( "Final counter value: %d (Expected: %d)\n" ,
             shared_counter , THREADS * OPERATIONS );
    return 0;
}


int lock_manager_test_main ( ) {
    test_basic ( );
    test_concurrent ( );
    test_edge_cases ( );
    pthread_lock_test ( );
    printf ( "All tests passed!\n" );
    return 0;
}
