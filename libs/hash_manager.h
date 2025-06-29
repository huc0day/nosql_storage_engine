#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

//哈希表实现

#define TABLE_SIZE 1048577  //哈希表大小（与 1048576 最相近的 质数，数学特性：2^20 + 1）
#define KEY_NOT_FOUND -1    //错误代码（KEY键未找到：-1）


//唯一索引哈希表节点结构（使用开放寻址法处理冲突，存储键值对(char*, int，)，表大小为1048576(2^20)，增加了删除标记位(is_deleted)实现惰性删除）
typedef struct {
    char * key;
    int value;
    bool is_deleted;  // 标记删除状态
} UniqueEntry;

extern UniqueEntry table[TABLE_SIZE];

//普通索引哈希表节点结构
typedef struct IndexEntry {
    uint64_t key;
    void              * data;
    struct IndexEntry * next;
} IndexEntry;

extern IndexEntry * hash_table[TABLE_SIZE];

//计算字符串键的哈希值
unsigned int hash_unique ( const char * key );

//计算uint64_t键的哈希值
uint32_t hash_from_uint64_to_uint32 ( uint64_t key );

//插入数据
int insert_unique ( const char * key , int value );

//查询数据
int query_unique ( const char * key );

//删除数据
bool delete_unique ( const char * key );

//向IndexEntry表插入数据
void insert_index ( uint64_t key , void * data );

//从IndexEntry表查询数据
void * query_index ( uint64_t key );

//从IndexEntry表删除数据
bool delete_index ( uint64_t key );

#endif
