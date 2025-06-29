#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "index_manager.h"

//创建唯一索引管理器
unique_index_manager * create_unique_index_manager ( char * name ) {
    //
    if ( name == NULL ) { return NULL; }
    //
    unique_index_manager * u_i_m = malloc ( sizeof ( unique_index_manager ) );
    if ( u_i_m == NULL ) {
        return NULL;
    }
    //
    memset ( u_i_m , 0 , sizeof ( unique_index_manager ) );
    //
    bool                 ret = init_unique_index_manager ( u_i_m , name );
    if ( ret != true ) {
        free ( u_i_m );
        return NULL;
    }
    //
    return u_i_m;
}

//初始化唯一索引管理器
bool init_unique_index_manager ( unique_index_manager * u_i_m , char * name ) {
    if ( name == NULL ) { return false; }
    if ( u_i_m != NULL ) {
        size_t name_len = strnlen ( name , 255 );
        u_i_m->name = malloc ( name_len + 1 );
        memset ( u_i_m->name , 0 , ( name_len + 1 ) );
        strncpy ( u_i_m->name , name , name_len );
        if ( u_i_m->root == NULL ) {
            u_i_m->root = calloc ( 1 , sizeof ( unique_index_key_node ) );
            if ( u_i_m->root == NULL ) {
                free ( u_i_m->name );
                return false;
            }
            u_i_m->count = 0;
        }
        return true;
    }
    return false;
}

//获得唯一索引管理器
unique_index_manager * get_unique_index_manager ( unique_index_manager * u_i_m , char * name ) {
    if ( u_i_m != NULL ) {
        return u_i_m;
    }
    if ( name == NULL ) {
        return NULL;
    }
    u_i_m = create_unique_index_manager ( name );
    return u_i_m;
}

//内部方法（外部不可见）（释放内部节点资源）
static void _destroy_unique_index_key_node ( unique_index_key_node * node ) {
    if ( node == NULL ) return;
    for ( int i = 0 ; i < CHAR_INDEX_CHILDREN_COUNT ; i++ ) {
        _destroy_unique_index_key_node ( node->children[ i ] );
    }
    if ( node->row_id_tree != NULL ) {
        free_index_tree ( node->row_id_tree );
    }
    free ( node );
}

//释放唯一索引管理器
void free_unique_index_manager ( unique_index_manager * u_i_m ) {
    if ( u_i_m != NULL ) {
        if(u_i_m->root!=NULL){
            _destroy_unique_index_key_node ( u_i_m->root );
        }
        if ( u_i_m->name != NULL ) {
            free ( u_i_m->name );
        }
        u_i_m->root  = NULL;
        u_i_m->count = 0;
        free ( u_i_m );
    }
}

//字符到索引的转换
int unique_index_char_to_index ( char c ) {
    if ( c >= 'a' && c <= 'z' ) return c - 'a';
    if ( c >= '0' && c <= '9' ) return 26 + ( c - '0' );
    return 36; // 下划线
}

//检测全部子节点是否均为空节点
bool unique_index_is_leaf ( unique_index_key_node * node ) {
    for ( int i = 0 ; i < CHAR_INDEX_CHILDREN_COUNT ; i++ ) {
        if ( node->children[ i ] != NULL ) return false;
    }
    return true;
}

//内部方法（外部不可见）插入子节点
static bool _insert_unique_index_key_node ( unique_index_key_node * node , const char * key ,
                                            index_row_id_key_node * row_id_tree , unsigned level ) {
    if ( row_id_tree == NULL ) { return false; }
    if ( * key == '\0' ) {
        if ( node->row_id_tree != NULL ) {
            // 键已存在
            printf (
                    "\nInserting unique key : char ( %c ) ascii ( %d ) , level ( %d ) , row_id_tree->id ( %llu ) , data insertion failed! The data already exists!\n" ,
                    ( * key ) ,
                    ( * key ) , level ,
                    ( unsigned long long ) node->row_id_tree->id );
            return false;
        }
        node->row_id_tree = row_id_tree;
        node->level       = level;
        printf (
                "\nInserting unique key : char ( %c ) ascii ( %d ) , level ( %d ) , row_id_tree->id ( %llu ) , Inserting data, data insertion successful!\n" ,
                ( * key ) ,
                ( * key ) ,
                level ,
                ( unsigned long long ) node->row_id_tree->id );
        return true;
    }

    int idx = unique_index_char_to_index ( * key );
    if ( node->children[ idx ] == NULL ) {
        node->children[ idx ] = calloc ( 1 , sizeof ( unique_index_key_node ) );
        node->children[ idx ]->key_char = * key;
    }

    return _insert_unique_index_key_node ( node->children[ idx ] , key + 1 , row_id_tree , level + 1 );
}

//添加唯一索引
bool insert_unique_index_key ( unique_index_manager * u_i_m , const char * key , index_row_id_key_node * row_id_tree ) {
    printf ( "\nstart insert unique key : key ( %s ) , row_id_tree->id ( %llu )\n" , key ,
             ( unsigned long long ) row_id_tree->id );
    return _insert_unique_index_key_node ( u_i_m->root , key , row_id_tree , 0 );
}

//根据索引查找行ID集合
index_row_id_key_node * search_unique_index_key ( unique_index_manager * u_i_m , const char * key ) {
    unique_index_key_node * current = u_i_m->root;
    while ( * key != '\0' ) {
        int idx = unique_index_char_to_index ( * key );
        if ( current->children[ idx ] == NULL ) return NULL;
        current = current->children[ idx ];
        key++;
    }
    return current->row_id_tree;
}

//内部方法（外部不可见）删除唯一索引节点
static bool _delete_unique_index_key_node ( unique_index_key_node * node , const char * key ) {
    //字符串索引的结束符
    if ( * key == '\0' ) {
        //销毁行ID集合占用的内存空间
        free_index_tree ( node->row_id_tree );
        return true;
    }

    int idx = unique_index_char_to_index ( * key );
    if ( node->children[ idx ] == NULL ) return false;

    bool result = _delete_unique_index_key_node ( node->children[ idx ] , key + 1 );
    if ( result == true && unique_index_is_leaf ( node->children[ idx ] ) ) {
        free ( node->children[ idx ] );
        node->children[ idx ] = NULL;
    }
    return result;
}

//删除唯一索引
bool delete_unique_index_key ( unique_index_manager * u_i_m , const char * key ) {
    return _delete_unique_index_key_node ( u_i_m->root , key );
}


////////////////////////////////////////////////////////////////////////////////////////////////////

// 创建唯一索引节点
unique_index_key_node * create_unique_index_node ( char key_char , unsigned level ) {
    //
    unique_index_key_node * node = malloc ( sizeof ( unique_index_key_node ) );
    if(node!=NULL){
        //
        node->key_char    = key_char;
        node->level       = level;
        node->row_id_tree = NULL;
        //
        memset ( node->children , 0 , sizeof ( node->children ) );
    }
    //
    return node;
}

// 销毁唯一索引节点
void free_unique_tree ( unique_index_key_node * node ) {
    if ( node == NULL ) return;

    for ( int i = 0 ; i < CHAR_INDEX_CHILDREN_COUNT ; i++ ) {
        free_unique_tree ( node->children[ i ] );
    }

    if ( node->row_id_tree ) {
        free ( node->row_id_tree );
    }

    free ( node );
}


// 向指定的KEY下面的行ID集合中添加行ID
bool insert_unique_index_key_for_row_id ( unique_index_manager * u_i_m , const char * key , size_t row_id ) {
    unique_index_key_node * current = u_i_m->root;

    for ( int i = 0 ; key[ i ] != '\0' ; i++ ) {
        int idx = unique_index_char_to_index ( key[ i ] );
        if ( idx == -1 ) return false;

        if ( current->children[ idx ] == NULL ) {
            current->children[ idx ] = create_unique_index_node ( key[ i ] , current->level + 1 );
        }
        current = current->children[ idx ];
    }

    // 插入到行ID树
    if ( current->row_id_tree == NULL ) {
        current->row_id_tree = malloc ( sizeof ( index_row_id_key_node ) );
        memset ( current->row_id_tree , 0 , sizeof ( index_row_id_key_node ) );
    }

    // 这里可以调用之前实现的insert_row_id函数
    insert_row_id ( & ( current->row_id_tree ) , row_id );

    u_i_m->count++;
    return true;
}

//内部方法（外部不可见）删除唯一索引节点
static bool _delete_unique_index_key_node_by_row_id ( unique_index_key_node * node , const char * key,size_t row_id ) {

    //字符串索引的结束符
    if ( * key == '\0' ) {
        //销毁行ID集合占用的内存空间
        delete_row_id ( &(node->row_id_tree),row_id );
        //
        return true;
    }

    int idx = unique_index_char_to_index ( * key );
    if ( node->children[ idx ] == NULL ) {
        //
        return false;
    }
    bool result = _delete_unique_index_key_node_by_row_id ( node->children[ idx ] , key + 1 ,row_id);
    if ( result == true && unique_index_is_leaf ( node->children[ idx ] ) ) {
        //
        free ( node->children[ idx ] );
        node->children[ idx ] = NULL;
        //
    }
    //
    return result;
}

////删除唯一索引中的指定ID
bool delete_unique_index_key_by_row_id ( unique_index_manager * u_i_m , const char * key,size_t row_id ) {
    if(u_i_m==NULL){
        //
        return false;
    }
    if(key==NULL){
        //
        return false;
    }
    if(row_id<=0){
        //
        return false;
    }
    //
    return _delete_unique_index_key_node_by_row_id ( u_i_m->root , key,row_id );
}

// 遍历所有键（递归辅助函数）
static inline void safe_callback(const char *key,
                                 size_t row_id,
                                 void (*cb)(const char*, size_t)) {
    if(key && cb) cb(key, row_id);
}

void traverse_keys_helper(unique_index_key_node *node,
                          char *buffer, int depth,
                          void (*callback)(const char*, size_t)) {
    if(!node || depth > MAX_INDEX_LEVEL) return;

    buffer[depth] = node->key_char;

    if(node->row_id_tree) {
        buffer[depth+1] = '\0';
        safe_callback(buffer, node->row_id_tree->row_id, callback);
    }

    for(int i=0; i<CHAR_INDEX_CHILDREN_COUNT; ) {
        if(node->children[i]) {
            traverse_keys_helper(node->children[i], buffer,
                                 depth+1, callback);
        }
        if(++i >= CHAR_INDEX_CHILDREN_COUNT) break;
    }
}

void traverse_keys(unique_index_manager *u_i_m,
                   void (*callback)(const char*, size_t)) {
    if(!u_i_m || !callback) return;

    char buffer[MAX_INDEX_KEY_LEN] = {0};
    if(u_i_m->root) {
        traverse_keys_helper(u_i_m->root, buffer, 0, callback);
    }
}




// 创建新的普通索引节点
index_row_id_key_node * create_index_node ( char key_char , unsigned level ) {
    index_row_id_key_node * node = malloc ( sizeof ( index_row_id_key_node ) );
    node->key_char = key_char;
    node->level    = level;
    node->row_id   = 0;
    memset ( node->children , 0 , sizeof ( node->children ) );
    return node;
}

// 插入行ID（递归实现）
bool insert_row_id ( index_row_id_key_node ** root , size_t row_id ) {
    if ( * root == NULL ) {
        * root = create_index_node ( '\0' , 0 );
    }

    index_row_id_key_node * current = * root;
    size_t   temp       = row_id;
    unsigned digits[20] = { 0 };
    int      count      = 0;

    // 分解数字
    while ( temp > 0 ) {
        digits[ count++ ] = temp % 10;
        temp /= 10;
    }

    // 从最高位开始插入
    for ( int i = count - 1 ; i >= 0 ; i-- ) {
        unsigned digit = digits[ i ];
        if ( current->level >= MAX_INDEX_LEVEL ) return false;

        if ( current->children[ digit ] == NULL ) {
            current->children[ digit ] = create_index_node ( digit + '0' , current->level + 1 );
        }
        current = current->children[ digit ];
    }

    current->row_id = row_id;

    printf("\nunique_index insert row_id : %llu\n",( unsigned long long)row_id);

    return true;
}

// 删除行ID（递归实现）
bool delete_row_id ( index_row_id_key_node ** root , size_t row_id ) {
    if ( * root == NULL ) return false;

    index_row_id_key_node * current = * root;
    size_t   temp       = row_id;
    unsigned digits[20] = { 0 };
    int      count      = 0;

    // 分解数字
    while ( temp > 0 ) {
        digits[ count++ ] = temp % 10;
        temp /= 10;
    }

    // 查找路径
    index_row_id_key_node * path[MAX_INDEX_LEVEL] = { 0 };
    int path_len = 0;

    for ( int i = count - 1 ; i >= 0 ; i-- ) {
        unsigned digit = digits[ i ];
        if ( current->children[ digit ] == NULL ) return false;

        path[ path_len++ ] = current;
        current = current->children[ digit ];
    }

    // 标记删除
    current->row_id = 0;

    // 清理空节点（从叶子向根）
    for ( int i = path_len - 1 ; i >= 0 ; i-- ) {
        index_row_id_key_node * node = path[ i ];
        bool has_children = false;
        for ( int             j      = 0 ; j < 10 ; j++ ) {
            if ( node->children[ j ] != NULL &&
                 ( node->children[ j ]->row_id != 0 ||
                   has_non_empty_children ( node->children[ j ] ) ) ) {
                has_children = true;
                break;
            }
        }
        if ( !has_children ) {
            free ( node );
            node = NULL;
        }
    }
    printf("\nunique_index delete row_id : %llu ,\n",(
    unsigned long long)row_id);
    return true;
}

// 检查节点是否有非空子节点
bool has_non_empty_children ( index_row_id_key_node * node ) {
    for ( int i = 0 ; i < 10 ; i++ ) {
        if ( node->children[ i ] != NULL ) return true;
    }
    return false;
}

// 查找行ID是否存在
bool find_row_id ( index_row_id_key_node * root , size_t row_id ) {
    if ( root == NULL ) return false;

    index_row_id_key_node * current = root;
    size_t   temp       = row_id;
    unsigned digits[20] = { 0 };
    int      count      = 0;

    while ( temp > 0 ) {
        digits[ count++ ] = temp % 10;
        temp /= 10;
    }

    for ( int i = count - 1 ; i >= 0 ; i-- ) {
        unsigned digit = digits[ i ];
        if ( current->children[ digit ] == NULL ) return false;
        current = current->children[ digit ];
    }

    return current->row_id == row_id;
}

// 分页获取所有行ID（中序遍历）
void get_row_ids_paginated ( index_row_id_key_node * root , size_t ** results , int * total , int page , int * count ) {
    if ( root == NULL ) return;

    static int collected = 0;
    static int start     = 0;
    static int end       = 0;

    if ( page == 1 ) {
        collected = 0;
        start     = ( page - 1 ) * PAGE_SIZE;
        end       = page * PAGE_SIZE;
    }

    // 遍历子节点
    for ( int i = 0 ; i < 10 ; i++ ) {
        if ( root->children[ i ] != NULL ) {
            get_row_ids_paginated ( root->children[ i ] , results , total , page , count );
        }
    }

    // 收集当前节点的行ID
    if ( root->row_id != 0 ) {
        if ( collected >= start && collected < end ) {
            ( * results )[ ( * count )++ ] = root->row_id;
        }
        collected++;
        ( * total )++;
    }
}

// 初始化分页查询
void init_paginated_query ( size_t ** results , int * total , int * count ) {
    * results = malloc ( PAGE_SIZE * sizeof ( size_t ) );
    * total   = 0;
    * count   = 0;
}

// 清理索引树
void free_index_tree ( index_row_id_key_node * root ) {
    if ( root == NULL ) return;

    for ( int i = 0 ; i < 10 ; i++ ) {
        free_index_tree ( root->children[ i ] );
    }

    free ( root );
}
