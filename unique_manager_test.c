#include "libs/unique_manager.h"

//
int unique_manager_test_main ( ) {

    // 1. 初始化索引
    unique_index user_index;
    init_unique_index ( & user_index );

    // 2. 插入测试数据
    printf ( "插入操作:\n" );
    insert_unique_key ( & user_index , "user_1001" , 1001 );
    insert_unique_key ( & user_index , "user_1002" , 1002 );
    insert_unique_key ( & user_index , "admin" , 1 );

    // 3. 查询演示
    printf ( "\n查询操作:\n" );
    printf ( "admin -> %zu\n" , search_unique_key ( & user_index , "admin" ) );
    printf ( "user_1001 -> %zu\n" , search_unique_key ( & user_index , "user_1001" ) );
    printf ( "not_exist -> %zu\n" , search_unique_key ( & user_index , "not_exist" ) );

    // 4. 删除演示
    printf ( "\n删除操作:\n" );
    delete_unique_key ( & user_index , "user_1001" );
    printf ( "删除后查询 user_1001 -> %zu\n" , search_unique_key ( & user_index , "user_1001" ) );

    // 5. 内存清理
    destroy_unique_index ( & user_index );
    //
    return 0;
}


