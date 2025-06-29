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

#ifndef UNIQUE_MANAGER_H
#define UNIQUE_MANAGER_H

#include <stdio.h>
#include <stdbool.h>

#define MAX_UNIQUE_KEY_LEN 32
#define MAX_UNIQUE_LEVEL (MAX_INDEX_KEY_LEN-2)
#define CHAR_UNIQUE_CHILDREN_COUNT 37 // a-z0-9+_

//唯一索引节点树
typedef struct _index_key_node {
    size_t id;
    char   key_char;
    struct _index_key_node * children[CHAR_UNIQUE_CHILDREN_COUNT]; // a-z0-9+_
    size_t   row_id;
    unsigned level;
} unique_key_node;

//唯一索引树根节点
typedef struct _unique_manager {
    char            * name;
    unique_key_node * root;
    size_t count;
} unique_manager;

//
unique_manager * create_unique_manager ( char * name );

//
bool init_unique_manager ( unique_manager * u_m , char * name );

//
void free_unique_manager ( unique_manager * u_m );

//
int insert_unique_key ( unique_manager * u_m , const char * key , size_t row_id );

//
size_t search_unique_key ( unique_manager * u_m , const char * key );

//
int delete_unique_key ( unique_manager * u_m , const char * key );

#endif //UNIQUE_MANAGER_H
