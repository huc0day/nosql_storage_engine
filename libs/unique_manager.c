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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unique_manager.h"

//创建唯一索引管理器
unique_manager * create_unique_manager ( char * name ) {
    //
    if( name == NULL ){ return NULL; }
    //
    unique_manager * u_m = malloc ( sizeof ( unique_manager ) );
    if ( u_m == NULL ) {
        return NULL;
    }
    //
    memset ( u_m , 0 , sizeof ( unique_manager ) );
    //
    bool ret = init_unique_manager ( u_m , name );
    if(ret!=true){
        free(u_m);
        return NULL;
    }
    //
    return u_m;
}

//初始化唯一索引管理器
bool init_unique_manager ( unique_manager * u_m , char * name ) {
    if(name==NULL){ return false; }
    if(u_m!=NULL){
        size_t name_len = strnlen ( name , 255 );
        u_m->name = malloc ( name_len + 1 );
        memset(u_m->name,0,( name_len + 1 ));
        strncpy ( u_m->name , name , name_len );
        u_m->root  = calloc ( 1 , sizeof ( unique_key_node ) );
        if( u_m->root == NULL ){
            free( u_m->name );
            return false;
        }
        u_m->count = 0;
        return true;
    }
    return false;
}

//内部方法（外部不可见）（释放内部节点资源）
static void _destroy_unique_key_node ( unique_key_node * node ) {
    if ( node == NULL ) return;
    for ( int i = 0 ; i < CHAR_UNIQUE_CHILDREN_COUNT ; i++ ) {
        _destroy_unique_key_node ( node->children[ i ] );
    }
    free ( node );
}

//释放唯一索引管理器
void free_unique_manager ( unique_manager * u_m ) {
    if ( u_m != NULL ) {
        _destroy_unique_key_node ( u_m->root );
        if ( u_m->name != NULL ) {
            free ( u_m->name );
        }
        u_m->root  = NULL;
        u_m->count = 0;
        free ( u_m );
    }
}

//字符到索引
static int _char_to_index ( char c ) {
    if ( c >= 'a' && c <= 'z' ) return c - 'a';
    if ( c >= '0' && c <= '9' ) return 26 + ( c - '0' );
    return 36; // 下划线
}

//
static int _is_leaf ( unique_key_node * node ) {
    for ( int i = 0 ; i < CHAR_UNIQUE_CHILDREN_COUNT ; i++ ) {
        if ( node->children[ i ] != NULL ) return 0;
    }
    return 1;
}

//
static int _insert_unique_key_node ( unique_key_node * node , const char * key ,
                                     size_t row_id , unsigned level ) {
    if ( * key == '\0' ) {
        if ( node->row_id != 0 ) {
            // 键已存在
            printf (
                    "\nInserting unique key : char ( %c ) ascii ( %d ) , level ( %d ) , row_id ( %llu ) , data insertion failed! The data already exists!\n" ,
                    ( * key ) ,
                    ( * key ) , level ,
                    ( unsigned long long ) row_id );
            return -1;
        }
        node->row_id = row_id;
        node->level  = level;
        printf (
                "\nInserting unique key : char ( %c ) ascii ( %d ) , level ( %d ) , row_id ( %llu ) , Inserting data, data insertion successful!\n" ,
                ( * key ) ,
                ( * key ) , level ,
                ( unsigned long long ) row_id );
        return 0;
    }

    int idx = _char_to_index ( * key );
    if ( node->children[ idx ] == NULL ) {
        node->children[ idx ] = calloc ( 1 , sizeof ( unique_key_node ) );
        node->children[ idx ]->key_char = * key;
    }

    return _insert_unique_key_node ( node->children[ idx ] , key + 1 , row_id , level + 1 );
}

//添加唯一索引
int insert_unique_key ( unique_manager * u_m , const char * key , size_t row_id ) {
    printf ( "\nstart insert unique key : key ( %s ) , row_id ( %llu )\n" , key , ( unsigned long long ) row_id );
    return _insert_unique_key_node ( u_m->root , key , row_id , 0 );
}

//根据索引查找行ID
size_t search_unique_key ( unique_manager * u_m , const char * key ) {
    unique_key_node * current = u_m->root;
    while ( * key != '\0' ) {
        int idx = _char_to_index ( * key );
        if ( current->children[ idx ] == NULL ) return 0;
        current = current->children[ idx ];
        key++;
    }
    return current->row_id;
}

//
static int _delete_unique_key_node ( unique_key_node * node , const char * key ) {
    if ( * key == '\0' ) {
        size_t old_id = node->row_id;
        node->row_id = 0;
        return old_id ? 0 : -1;
    }

    int idx = _char_to_index ( * key );
    if ( node->children[ idx ] == NULL ) return -1;

    int result = _delete_unique_key_node ( node->children[ idx ] , key + 1 );
    if ( result == 0 && _is_leaf ( node->children[ idx ] ) ) {
        free ( node->children[ idx ] );
        node->children[ idx ] = NULL;
    }
    return result;
}

//删除唯一索引
int delete_unique_key ( unique_manager * u_m , const char * key ) {
    return _delete_unique_key_node ( u_m->root , key );
}
