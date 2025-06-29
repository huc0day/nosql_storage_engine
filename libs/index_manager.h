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

#ifndef INDEX_MANAGER_H
#define INDEX_MANAGER_H

#include <stdio.h>
#include <stdbool.h>

//
#define MAX_INDEX_KEY_LEN 32
#define MAX_INDEX_LEVEL (MAX_INDEX_KEY_LEN-2)
#define UNSIGNED_INDEX_CHILDREN_COUNT 10 // 0-9
#define CHAR_INDEX_CHILDREN_COUNT 37 // a-z0-9+_
#define PAGE_SIZE 10

typedef struct _index_row_id_key_node {
    size_t id;
    char   key_char; // 0-9
    struct _index_row_id_key_node * children[UNSIGNED_INDEX_CHILDREN_COUNT];
    size_t   row_id;
    unsigned level;
} index_row_id_key_node;

typedef struct _unique_index_key_node {
    size_t id;
    char   key_char; // a-z0-9+_
    struct _unique_index_key_node * children[CHAR_INDEX_CHILDREN_COUNT];
    index_row_id_key_node         * row_id_tree; // 指向10叉树根节点
    unsigned level;
    size_t   count;
} unique_index_key_node;

typedef struct _unique_index_manager {
    char * name;
    unique_index_key_node * root;
    size_t count;
} unique_index_manager;

// 管理器操作
//
//创建唯一索引管理器
unique_index_manager * create_unique_index_manager ( char * name ) ;

//初始化唯一索引管理器
bool init_unique_index_manager ( unique_index_manager * u_i_m , char * name ) ;

//获得唯一索引管理器
unique_index_manager * get_unique_index_manager ( unique_index_manager * u_i_m , char * name ) ;


//释放唯一索引管理器
void free_unique_index_manager ( unique_index_manager * u_i_m ) ;

//添加唯一索引
bool insert_unique_index_key ( unique_index_manager * u_i_m , const char * key , index_row_id_key_node * row_id_tree ) ;

//根据索引查找行ID集合
index_row_id_key_node * search_unique_index_key ( unique_index_manager * u_i_m , const char * key ) ;

//删除唯一索引
bool delete_unique_index_key ( unique_index_manager * u_i_m , const char * key );

////////////////////////////////////////////////////////////////////////////////////////////////////


// 字符到索引的转换
int unique_index_char_to_index ( char c ) ;

//检测全部子节点是否均为空节点
bool unique_index_is_leaf ( unique_index_key_node * node );

// 创建唯一索引节点
unique_index_key_node * create_unique_index_node ( char key_char , unsigned level ) ;

// 插入键值对
bool insert_unique_index_key_for_row_id ( unique_index_manager * u_i_m , const char * key , size_t row_id ) ;

//删除唯一索引中的指定ID
bool delete_unique_index_key_by_row_id ( unique_index_manager * u_i_m , const char * key,size_t row_id );

// 查找键对应的行ID
size_t find_by_key ( unique_index_manager * mgr , const char * key ) ;

// 删除键值对
bool delete_key ( unique_index_manager * u_i_m , const char * key ) ;

// 遍历所有键（递归辅助函数）
void traverse_keys_helper ( unique_index_key_node * node , char * buffer , int depth ,
                            void (* callback) ( const char * , size_t ) );

// 遍历所有键值对
void traverse_keys ( unique_index_manager * u_i_m , void (* callback) ( const char * , size_t ) ) ;

// 创建新节点
index_row_id_key_node * create_index_node ( char key_char , unsigned level );

// 插入行ID（递归实现）
bool insert_row_id ( index_row_id_key_node ** root , size_t row_id ) ;
// 删除行ID（递归实现）
bool delete_row_id ( index_row_id_key_node ** root , size_t row_id );
// 检查节点是否有非空子节点
bool has_non_empty_children ( index_row_id_key_node * node ) ;
// 查找行ID是否存在
bool find_row_id ( index_row_id_key_node * root , size_t row_id ) ;

// 分页获取所有行ID（中序遍历）
void get_row_ids_paginated ( index_row_id_key_node * root , size_t ** results , int * total , int page , int * count );

// 初始化分页查询
void init_paginated_query ( size_t ** results , int * total , int * count ) ;
// 清理索引树
void free_index_tree ( index_row_id_key_node * root ) ;

//
#endif //INDEX_MANAGER_H
