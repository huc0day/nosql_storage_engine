#include "libs/hash_manager.h"

// 测试哈希函数
void test_hash_unique_function ( ) {
    printf ( "Testing hash_unique function...\n" );
    assert( hash_unique ( "abc" ) == hash_unique ( "abc" ) );  // 相同输入相同输出
    assert( hash_unique ( "abc" ) != hash_unique ( "cba" ) );  // 不同输入不同输出
}

// 测试插入功能
void test_insert_unique ( ) {
    printf ( "Testing insert...\n" );
    assert( insert_unique ( "key1" , 100 ) == 0 );  // 正常插入
    assert( insert_unique ( "key1" , 200 ) == -1 ); // 重复键
    assert( query_unique ( "key1" ) == 100 );      // 验证插入值
}

// 测试查询功能
void test_query_unique ( ) {
    printf ( "Testing query...\n" );
    assert( query_unique ( "nonexist" ) == KEY_NOT_FOUND ); // 不存在的键
    insert_unique ( "key2" , 200 );
    assert( query_unique ( "key2" ) == 200 );  // 存在键查询
}

// 测试删除功能
void test_delete_unique ( ) {
    printf ( "Testing delete...\n" );
    insert_unique ( "key3" , 300 );
    assert( delete_unique ( "key3" ) == true );    // 正常删除
    assert( query_unique ( "key3" ) == KEY_NOT_FOUND ); // 验证删除
    assert( delete_unique ( "key3" ) == false );   // 重复删除
}

// 测试冲突处理
void test_collision_unique ( ) {
    printf ( "Testing collision...\n" );
    // 模拟两个不同键哈希到同一位置
    char key_a[10] = { 0 } , key_b[10] = { 0 };
    snprintf ( key_a , sizeof ( key_a ) , "%c%c" , 1 , 1 );
    snprintf ( key_b , sizeof ( key_b ) , "%c%c" , 17 , 17 );

    if ( hash_unique ( key_a ) == hash_unique ( key_b ) ) {
        insert_unique ( key_a , 1 );
        assert( insert_unique ( key_b , 2 ) == 0 ); // 冲突时应能插入
        assert( query_unique ( key_b ) == 2 );     // 冲突后能查询
    }
}

// 主测试函数（唯一索引）
int test_unique_main ( ) {
    // 初始化哈希表
    memset ( table , 0 , sizeof ( table ) );

    test_hash_unique_function ( );
    test_insert_unique ( );
    test_query_unique ( );
    test_delete_unique ( );
    test_collision_unique ( );

    printf ( "All tests passed!\n" );
    return 0;
}

// 测试哈希函数
void test_hash_index_function ( ) {
    printf ( "Testing hash_unique function...\n" );
    // 验证均匀分布
    assert( hash_from_uint64_to_uint32 ( 0 ) == 0 );
    assert( hash_from_uint64_to_uint32 ( TABLE_SIZE - 1 ) == TABLE_SIZE - 1 );
    assert( hash_from_uint64_to_uint32 ( TABLE_SIZE ) == 0 );  // 模运算验证
}

// 测试基础CRUD操作
void test_index_basic_operations ( ) {
    printf ( "Testing basic operations...\n" );
    int test_data1 = 42;
    int test_data2 = 99;

    // 测试插入和查询
    insert_index ( 123 , & test_data1 );
    assert( * ( int * ) query_index ( 123 ) == 42 );

    // 测试更新
    insert_index ( 123 , & test_data2 );  // 同key覆盖
    assert( * ( int * ) query_index ( 123 ) == 99 );

    // 测试删除
    assert( delete_index ( 123 ) == true );
    assert( query_index ( 123 ) == NULL );
    assert( delete_index ( 123 ) == false );  // 重复删除
}

// 测试冲突处理
void test_index_collision_handling ( ) {
    printf ( "Testing collision handling...\n" );
    char * str1 = "collision1";
    char * str2 = "collision2";

    // 制造冲突（假设TABLE_SIZE足够大）
    uint64_t key1 = 100;
    uint64_t key2 = 100 + TABLE_SIZE;

    insert_index ( key1 , str1 );
    insert_index ( key2 , str2 );

    assert( query_index ( key1 ) == str1 );
    assert( query_index ( key2 ) == str2 );

    // 验证链表结构
    uint32_t slot = hash_from_uint64_to_uint32 ( key1 );
    assert( hash_table[ slot ] != NULL );
    assert( hash_table[ slot ]->next != NULL );
}

// 测试内存管理
void test_index_memory_management ( ) {
    printf ( "Testing memory management...\n" );
    void * data = malloc ( 10 );
    insert_index ( 456 , data );
    assert( delete_index ( 456 ) == true );  // 应释放data内存
    // 注意：实际需要valgrind等工具检测内存泄漏
}

// 主测试函数（普通索引）
int test_index_main ( ) {
    // 初始化哈希表
    for ( int i = 0 ; i < TABLE_SIZE ; i++ ) {
        hash_table[ i ] = NULL;
    }

    test_hash_index_function ( );
    test_index_basic_operations ( );
    test_index_collision_handling ( );
    test_index_memory_management ( );

    printf ( "All tests passed!\n" );
    return 0;
}

// 主测试函数
int test_hash_manager_main ( ) {
    test_unique_main ( );
    test_index_main ( );
    return 0;
}
