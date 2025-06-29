#include "hash_manager.h"

//唯一索引哈希表节点集合
UniqueEntry table[TABLE_SIZE];

//普通索引哈希表节点集合
IndexEntry * hash_table[TABLE_SIZE];

//
// 哈希函数
unsigned int hash_unique ( const char * key ) {
    uint64_t hashval = 5381;
    while ( * key ) {
        hashval = ( ( hashval << 5 ) + hashval ) + * key++;
    }
    return ( uint32_t ) ( hashval % TABLE_SIZE );
}

// 带冲突处理的插入函数
int insert_unique ( const char * key , int value ) {
    unsigned int idx       = hash_unique ( key );
    int          start_idx = idx;

    do {
        // 空槽或已删除的槽
        if ( table[ idx ].key == NULL || table[ idx ].is_deleted ) {
            if ( table[ idx ].key ) free ( table[ idx ].key );
            table[ idx ].key        = strdup ( key );
            table[ idx ].value      = value;
            table[ idx ].is_deleted = false;
            return 0;
        }
        // 键已存在
        if ( strcmp ( table[ idx ].key , key ) == 0 ) {
            return -1;
        }
        // 线性探测
        idx = ( idx + 1 ) % TABLE_SIZE;
    } while ( idx != start_idx );

    return -2; // 表已满
}

// 带冲突处理的查询函数
int query_unique ( const char * key ) {
    unsigned int idx       = hash_unique ( key );
    int          start_idx = idx;

    do {
        // 找到有效且匹配的键
        if ( table[ idx ].key != NULL && !table[ idx ].is_deleted &&
             strcmp ( table[ idx ].key , key ) == 0 ) {
            return table[ idx ].value;
        }
        // 遇到空槽且未被删除过，说明键不存在
        if ( table[ idx ].key == NULL && !table[ idx ].is_deleted ) {
            break;
        }
        idx = ( idx + 1 ) % TABLE_SIZE;
    } while ( idx != start_idx );

    return KEY_NOT_FOUND;
}

// 删除函数
bool delete_unique ( const char * key ) {
    unsigned int idx       = hash_unique ( key );
    int          start_idx = idx;

    do {
        if ( table[ idx ].key != NULL && !table[ idx ].is_deleted &&
             strcmp ( table[ idx ].key , key ) == 0 ) {
            table[ idx ].is_deleted = true;
            return true;
        }
        if ( table[ idx ].key == NULL && !table[ idx ].is_deleted ) {
            break;
        }
        idx = ( idx + 1 ) % TABLE_SIZE;
    } while ( idx != start_idx );

    return false;
}

//
uint32_t hash_from_uint64_to_uint32 ( uint64_t key ) {
    return ( uint32_t ) ( key % TABLE_SIZE );
}

//
void insert_index ( uint64_t key , void * data ) {
    uint32_t slot = hash_from_uint64_to_uint32 ( key );
    IndexEntry * entry = malloc ( sizeof ( IndexEntry ) );
    entry->key  = key;
    entry->data = data;
    entry->next = hash_table[ slot ];
    hash_table[ slot ] = entry;
}

//
void * query_index ( uint64_t key ) {
    IndexEntry * entry = hash_table[ hash_from_uint64_to_uint32 ( key ) ];
    while ( entry && entry->key != key ) {
        entry = entry->next;
    }
    return entry ? entry->data : NULL;
}

//
bool delete_index(uint64_t key) {
    uint32_t slot = hash_from_uint64_to_uint32(key);
    IndexEntry **pp_entry = &hash_table[slot];  // 二级指针用于修改链表
    IndexEntry *entry = *pp_entry;

    while (entry) {
        if (entry->key == key) {
            *pp_entry = entry->next;  // 绕过当前节点
            free(entry->data);        // 释放数据内存（根据需求可选）
            free(entry);              // 释放节点内存
            return true;
        }
        pp_entry = &entry->next;     // 移动到下一个指针位置
        entry = entry->next;
    }
    return false;  // 未找到键
}
