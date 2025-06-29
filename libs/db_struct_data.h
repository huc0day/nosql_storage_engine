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

#ifndef DB_STRUCT_DATA_H
#define HASH_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
#define ALIGNED_ALLOC(size) _aligned_malloc(size, 16)
#else
#define ALIGNED_ALLOC( size ) aligned_alloc(16, size)
#endif

//
#define DATA_SERVER_MAX_SIZE 1048576
#define DATA_BASE_MAX_SIZE   1048576
#define DATA_TABLE_MAX_SIZE  1048576
#define DATA_COL_MAX_SIZE  1024
#define DATA_COL_NAME_MAX_LEN 255
#define MAX_KEY_LEN 32
#define MAX_DEPTH 20
#define DATA_COL_VALUE_TYPE_NUMBER 1
#define DATA_COL_VALUE_TYPE_STRING 2
#define DATA_COL_VALUE_TYPE_BIN    3
//
#include "hybrid.h"
#include "unique_manager.h"
#include "index_manager.h"
#include "lock_manager.h"
//

//数据管理器
typedef struct _data_manager {
    struct _data_server * data_servers[DATA_SERVER_MAX_SIZE]; //数据服务器集合
} data_manager , * data_manager_point;

//数据服务器
typedef struct _data_server {
    size_t id;                               //服务器ID
    char * key;                              //服务器名称
    size_t ipv4;                             //服务器IP地址
    size_t port;                             //服务器端口号
    struct _data_base * data_bases[DATA_BASE_MAX_SIZE]; //数据库集合
} data_server , * data_server_point;

//数据库
typedef struct _data_base {
    size_t id;                                 //数据库ID
    char               * key;                              //数据库名称
    struct _data_table * data_tables[DATA_TABLE_MAX_SIZE]; //数据表集合
} data_base , * data_base_point;

//数据表
typedef struct _data_table {
    size_t id;                                                    //数据库表ID
    char                   * key;                                 //数据表名称
    struct _branch_node    * root_branch_node;                    //根分支节点
    struct _unique_manager * unique_managers[DATA_COL_MAX_SIZE];  //表字段唯一索引集合
    struct _unique_index_manager  * unique_index_managers[DATA_COL_MAX_SIZE];   //表字段普通索引集合
} data_table , * data_table_point;

//数据列
typedef struct _data_col {
    size_t row_id;
    char * name;
    size_t value_type;
    size_t value_size;
    bool value_unique;
    void * value;
} data_col;

//数据行
typedef struct _data_row {
    size_t id;
    struct _data_row    * previous_node;
    struct _data_row    * next_node;
    struct _branch_node * parent;
    struct _data_col    * cols[DATA_COL_MAX_SIZE];
    size_t fields_count;
} data_row , * data_row_point;

//二叉树节点
typedef struct _branch_node {
    size_t id;
    data_table * table;
    size_t row_id;
    size_t size;
    struct _branch_node * left_node;
    size_t left_node_start;
    size_t left_node_end;
    struct _branch_node * right_node;
    size_t right_node_start;
    size_t right_node_end;
    struct _data_row * row;
} branch_node , * branch_node_point;


//
// unique key table ( server_name,database_name,table_name,field_key,id )
// index key table ( server_name,database_name,table_name,field_key,id )

//初始化根二叉树
branch_node * create_branch_node_tree ( size_t size , data_table * table );

//初始化子二叉树
branch_node * create_subtree ( size_t start , size_t end , int is_left_subtree , data_table * table );

//扩容根二叉树（扩容量，是原容量的两倍）
branch_node * expand_branch_node_tree ( branch_node * b_n );

//释放跟二叉树及下级等所有资源
void free_branch_node_tree ( branch_node * root );

//备用方法（暂不使用）
void attach_data_to_leaves ( branch_node * node );

//创建数据列
data_col *
create_data_col ( char * field_name , void * value , size_t value_size ,
                  size_t value_type , bool value_unique );

//释放数据列
void free_data_col ( data_col * field );

//创建数据行
data_row * create_data_row ( size_t row_id , data_col * fields[] , size_t fields_count ,
                             branch_node * parent_node );

//释放数据行
void free_data_row ( data_row * row );

//检测数据是否存在（释放与target_id对应的row_id对应的row中是否存在数据）
int exist_data ( branch_node * node , size_t target_id , int depth );

//设置数据（设置与target_id对应的row_id对应的row中的数据）
int set_data (
        branch_node * node ,
        size_t target_id ,
        data_col
        * fields[] ,
        size_t fields_count ,
        int depth ,
        lock_manager_t * mgr
             );

//获得数据（获得与target_id对应的row_id对应的row中的数据）
branch_node * get_data ( branch_node * node , size_t target_id , int depth );

////删除数据（释放与target_id对应的row_id对应的row中的数据）
int delete_data ( branch_node * node , size_t target_id , int depth , lock_manager_t * mgr );

//打印二叉树节点（递归打印整个树的相关信息）
void print_branch_node_tree ( branch_node * node , int depth );

//打印全部行信息（递归打印全部行的相关信息）
void print_rows ( branch_node * node , int * count );

//打印指定行信息
void print_row ( const branch_node * node );

// 数据管理器实现

// 创建数据管理器（实例）
data_manager * create_data_manager ( );

// 销毁数据管理器
void free_data_manager ( data_manager * d_m );

//数据库服务器实现

//创建数据库服务器
data_server * create_data_server ( size_t server_id , const char * key , size_t ipv4 , size_t port );

//向数据管理器中添加数据库服务器
int add_data_server ( data_manager * mgr , data_server * server );

//在数据管理器中数据库服务器
data_server * find_data_server ( data_manager * mgr , size_t server_id );

//从数据管理器中移除数据库服务器
int remove_data_server ( data_manager * mgr , size_t server_id );

//数据库实现

// 创建数据库
data_base * create_data_base ( size_t db_id , const char * key );

//向数据库服务器中添加数据库
int add_data_base ( data_server * server , data_base * db );

//在数据库服务器中查找数据库
data_base * find_data_base ( data_server * server , size_t db_id );

//从数据库服务器中移除数据库
int remove_data_base ( data_server * server , size_t db_id );


//数据表实现

//创建数据表
data_table *
create_data_table ( size_t tb_id , const char * key , char * fd_names[] , unsigned fds_count );

//绑定根数据节点到数据表
data_table * bind_data_table_root_branch_node ( data_table * table , branch_node * root_branch_node );

//向数据库中添加数据表
int add_data_table ( data_base * db , data_table * table );

//从数据库中查找数据表
data_table * find_data_table ( data_base * db , size_t tb_id );

//从数据库中移除数据表
int remove_data_table ( data_base * db , size_t tb_id );


#endif
