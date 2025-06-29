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

// 链表结构 头文件 linked_list.h
#ifndef LINKED_LIST_H  // 头文件保护宏
#define LINKED_LIST_H

#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include "data_type.h" //载入自定义的数据类型头文件

#define LINKED_LIST_LEN sizeof(struct LINKED_LIST) //结构体大小
#define LINKED_LIST_PAGE_SIZE 10000                //分页大小
#define LINKED_LIST_PAGE_NAME_SIZE 32              //分页名称大小

//双向链表
typedef struct LINKED_LIST {
    struct LINKED_LIST * parent_point;     //父节点指针
    struct LINKED_LIST * child_head_point; //子链表中头节点的指针
    struct LINKED_LIST * last_point;       //上一条记录的指针
    struct LINKED_LIST * next_point;       //下一条记录的指针
    ID         id;                         //记录ID
    STRING     name;                       //记录KEY
    COMMON_MEMORY data;                    //记录数据
    SIZE       size;                       //记录数据大小
} LINKED_LIST , * LINKED_LIST_POINT;

// 生成自增ID的函数（线程安全）
unsigned long long generate_threadsafe_linked_id ( ) ;

// 生成自增ID的函数（线程安全）
unsigned long long generate_threadsafe_pages_id ( ) ;

//向链表中新增一条记录（私有方法，不可在外部调用）
LINKED_LIST_POINT _insertLinkedList (
        LINKED_LIST_POINT parent_point , //父节点指针
        LINKED_LIST_POINT last_point ,   //上一条记录的指针
        INDEX id ,                        //当前记录的ID数值
        STRING name ,                    //当前记录的KEY
        COMMON_MEMORY data ,             //当前记录的值
        SIZE size                        //当前记录的值大小
                                    ) ;
//更新链表中指定记录的值
ID updateLinkedList ( LINKED_LIST_POINT this_point , COMMON_MEMORY data , SIZE size ) ;

//删除链表中的记录
int deleteLinkedList ( LINKED_LIST_POINT this_point ) ;

//根据ID查找记录（正序查找）
LINKED_LIST_POINT findByIdInASC ( LINKED_LIST_POINT head_point , ID id ) ;

//根据ID查找记录（倒序查找）
LINKED_LIST_POINT findByIdInDESC ( LINKED_LIST_POINT foot_point , ID id ) ;

char * getName ( char * name ) ;

char * getStringData ( char * data ) ;

//新增数据
LINKED_LIST_POINT insertData ( char * t_name , void * data , unsigned long long data_size , LINKED_LIST_POINT * page_head_point ) ;

//根据ID采取分页方式查找记录
LINKED_LIST_POINT findById ( ID id , LINKED_LIST_POINT page_head_point ) ;

#endif
