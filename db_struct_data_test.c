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


#include "libs/db_struct_data.h"

int db_struct_data_test_main ( ) {
    
    //
    //创建锁管理器
    lock_manager_t * mgr = create_lock_manager ( );
    //
    //
    // 初始化树
    size_t branch_node_tree_size = 1024;
    size_t table_id              = 1;
    char * fd_names[DATA_COL_MAX_SIZE];
    memset ( fd_names , 0 , ( sizeof ( char * ) * DATA_COL_MAX_SIZE ) );
    for ( unsigned fds_index = 0 ; fds_index < DATA_COL_MAX_SIZE ; fds_index++ ) {
        fd_names[ fds_index ] = malloc ( DATA_COL_NAME_MAX_LEN + 1 );
        memset ( fd_names[ fds_index ] , 0 , ( DATA_COL_NAME_MAX_LEN + 1 ) );
        snprintf ( fd_names[ fds_index ] , DATA_COL_NAME_MAX_LEN , "field_%d" , fds_index );
    }
    //
    data_manager * d_m  = create_data_manager ( );
    data_server  * d_s  = create_data_server ( 1 , "server_1" , 2130706433 , 13306 );
    data_base    * d_b  = create_data_base ( 1 , "db_1" );
    data_table   * d_t  = create_data_table ( table_id , "tb_1" , fd_names , DATA_COL_MAX_SIZE );
    branch_node  * root = create_branch_node_tree ( branch_node_tree_size , d_t );
    if ( !root ) {
        fprintf ( stderr , "Failed to initialize tree\n" );
        return EXIT_FAILURE;
    }
    bind_data_table_root_branch_node ( d_t , root );
    //
    // 准备测试数据
    data_col * fields[DATA_COL_MAX_SIZE] = { 0 };
    for ( size_t index = 0 ; index < DATA_COL_MAX_SIZE ; index++ ) {
        char name[32];
        snprintf ( name , sizeof ( name ) , "field_%zu" , index );
        char * value = malloc ( 256 );
        memset ( value , 0 , 256 );
        snprintf ( value , 255 , "key_%llu" , ( unsigned long long ) ( rand ( ) % 100 ) );
        fields[ index ] = create_data_col ( name , value , 32 , DATA_COL_VALUE_TYPE_STRING , (((index>=2)&&((index%2)==0))?true:false) );
        if ( !fields[ index ] ) {
            fprintf ( stderr , "Failed to create column %zu\n" , index );
            goto cleanup;
        }
    }
    //
    // 设置节点数据
    size_t row_id = 10;
    set_data ( root , row_id , fields , DATA_COL_MAX_SIZE , 0 , mgr );
    branch_node * b_n_m = get_data ( root , row_id , 0 );
    print_row ( b_n_m );
    delete_data ( root , 10 , 0 , mgr );
    if ( !exist_data ( root , 10 , 0 ) ) {
        printf ( "\nThe record corresponding to row ID ( %llu ) has no data\n" , (
                unsigned long long ) row_id );
    }
    set_data ( root , row_id , fields , DATA_COL_MAX_SIZE , 0 , mgr );
    b_n_m = get_data ( root , row_id , 0 );
    print_row ( b_n_m );
    delete_data ( root , 10 , 0 , mgr );
    if ( !exist_data ( root , 10 , 0 ) ) {
        printf ( "\nThe record corresponding to row ID ( %llu ) has no data\n" , (
                unsigned long long ) row_id );
    }
    //
    add_data_table ( d_b , d_t );
    add_data_base ( d_s , d_b );
    add_data_server ( d_m , d_s );
    //
    cleanup:
    free_branch_node_tree ( root );
    remove_data_table ( d_b , d_t->id );
    remove_data_base ( d_s , d_b->id );
    remove_data_server ( d_m , d_s->id );
    free_data_manager ( d_m );

    return EXIT_SUCCESS;
}
