#include "db_struct_data.h"

//pthread_mutex_t global_lock_mutex; // 保护锁表的互斥锁
//hash_table_t *key_lock_table;      // KEY为键，锁状态为值

//当 create_branch_node_tree (8) 时
//第一次递归结果：    左侧节点（left.start=1,left.end=4），右侧节点（right.start=5,right.end=8）
//第二次递归结果：    左侧节点（left.start=1,left.end=2,right.start=3,right.end=4），右侧节点（left.start=5,left.end=6，right.start=7,right.end=8）
//
//
// start,end

// 静态变量：线程安全的自增ID
static size_t          node_next_id       = 1;           // 初始值为1
static pthread_mutex_t node_next_id_mutex = PTHREAD_MUTEX_INITIALIZER;  // 静态初始化互斥锁

// 生成自增ID的函数（线程安全）
size_t create_node_id ( ) {
    //
    size_t current_id = 0;
    //
    if ( pthread_mutex_lock ( & node_next_id_mutex ) != 0 ) {
        return SIZE_MAX; // 使用特殊错误值
    }
    //
    if ( node_next_id == SIZE_MAX ) {
        pthread_mutex_unlock ( & node_next_id_mutex );
        return SIZE_MAX; // 溢出错误
    }
    //
    current_id = node_next_id++;
    //
    pthread_mutex_unlock ( & node_next_id_mutex );
    //
    return current_id;
}

//初始化二叉树的子节点空间
branch_node * create_subtree ( size_t start , size_t end , int is_left_subtree , data_table * table ) {
    if ( end - start < 1 ) {
        //
        if ( end != start ) {
            return NULL;
        }
        //
        size_t new_node_id = create_node_id ( );
        if ( new_node_id >= SIZE_MAX ) {
            return NULL;
        }
        //
        branch_node * node = malloc ( sizeof ( branch_node ) );
        memset ( node , 0 , sizeof ( branch_node ) );
        node->id               = new_node_id;
        node->left_node_start  = 0;
        node->left_node_end    = 0;
        node->right_node_start = 0;
        node->right_node_end   = 0;
        node->left_node        = NULL;
        node->right_node       = NULL;
        node->row_id           = start;
        node->row              = NULL;
        node->size             = 1;
        node->table            = table;
        //
        printf ( "node_id : %llu , node_size : %llu , Range: %llu-%llu | L: %llu-%llu | R: %llu-%llu" ,
                 ( unsigned long long ) node->id , ( unsigned long long ) node->size , ( unsigned long long ) start ,
                 ( unsigned long long ) end , ( unsigned long long ) node->left_node_start ,
                 ( unsigned long long ) node->left_node_end ,
                 ( unsigned long long ) node->right_node_start , ( unsigned long long ) node->right_node_end );
        printf ( " | row_id : %llu" , ( unsigned long long ) node->row_id );
        printf ( " | row : %p" , node->row );
        printf ( "\n" );
        //
        return node;
    }

    size_t new_node_id = create_node_id ( );
    if ( new_node_id >= SIZE_MAX ) {
        return NULL;
    }
    //
    branch_node * node = malloc ( sizeof ( branch_node ) );
    memset ( node , 0 , sizeof ( branch_node ) );
    node->id = new_node_id;

    size_t mid = start + ( end - start ) / 2;
    node->left_node_start  = start;
    node->left_node_end    = mid;
    node->right_node_start = mid + 1;
    node->right_node_end   = end;
    node->size             = end;
    node->table            = table;

    printf ( "node_id : %llu , node_size : %llu , Range: %llu-%llu | L: %llu-%llu | R: %llu-%llu\n" ,
             ( unsigned long long ) node->id , ( unsigned long long ) node->size , ( unsigned long long ) start ,
             ( unsigned long long ) end , ( unsigned long long ) node->left_node_start ,
             ( unsigned long long ) node->left_node_end ,
             ( unsigned long long ) node->right_node_start , ( unsigned long long ) node->right_node_end );

    node->left_node  = create_subtree ( start , mid , is_left_subtree , table );
    node->right_node = create_subtree ( mid + 1 , end , is_left_subtree , table );
    return node;
}

//首次初始化存储空间
branch_node * create_branch_node_tree ( size_t size , data_table * table ) {
    if ( ( size < 2 ) || ( ( size % 2 ) != 0 ) ) return NULL;
    //
    size_t new_node_id = create_node_id ( );
    if ( new_node_id >= SIZE_MAX ) {
        return NULL;
    }
    //
    branch_node * root = malloc ( sizeof ( branch_node ) );
    memset ( root , 0 , sizeof ( branch_node ) );
    root->id    = new_node_id;
    root->size  = size;
    root->table = table;

    size_t mid = size / 2;
    root->left_node_start  = 1;
    root->left_node_end    = mid;
    root->right_node_start = mid + 1;
    root->right_node_end   = size;

    printf ( "node_id : %llu , node_size : %llu , Range: %llu-%llu | L: %llu-%llu | R: %llu-%llu\n" ,
             ( unsigned long long ) root->id , ( unsigned long long ) root->size ,
             ( unsigned long long ) root->left_node_start ,
             ( unsigned long long ) root->right_node_end ,
             ( unsigned long long ) root->left_node_start ,
             ( unsigned long long ) root->left_node_end ,
             ( unsigned long long ) root->right_node_start ,
             ( unsigned long long ) root->right_node_end );

    root->left_node  = create_subtree ( 1 , mid , 1 , table );    // 左子树
    root->right_node = create_subtree ( mid + 1 , size , 0 , table ); // 右子树
    //
    return root;
}

//扩容存储空间（扩容大小是当前空间大小的一倍）
branch_node * expand_branch_node_tree ( branch_node * b_n ) {
    //
    if ( ( b_n == NULL ) || ( ( b_n->size < 2 ) || ( ( b_n->size % 2 ) != 0 ) ) ) {
        return NULL;
    }
    //
    size_t new_node_id = create_node_id ( );
    if ( new_node_id >= SIZE_MAX ) {
        return NULL;
    }
    //
    branch_node * root = malloc ( sizeof ( branch_node ) );
    if ( root == NULL ) {
        return NULL;
    }
    memset ( root , 0 , sizeof ( branch_node ) );
    //
    root->id               = new_node_id;
    root->size             = ( b_n->size * 2 );
    root->left_node_start  = 1;
    root->left_node_end    = b_n->size;
    root->right_node_start = ( b_n->size + 1 );
    root->right_node_end   = root->size;
    root->table            = b_n->table;
    //
    root->left_node        = b_n;
    root->right_node       = create_subtree ( root->right_node_start , root->right_node_end , 0 , root->table );
    //
    return root;
}

//释放存储空间
void free_branch_node_tree ( branch_node * root ) {
    if ( root == NULL ) return;
    free_branch_node_tree ( root->left_node );
    free_branch_node_tree ( root->right_node );
    free_data_row ( root->row );

    free ( root );
}

//
void attach_data_to_leaves ( branch_node * node ) {
    if ( node == NULL ) return;

    // 判断是否为叶子节点
    if ( node->left_node == NULL && node->right_node == NULL ) {

        // 创建数据行实例
        data_row * new_row = malloc ( sizeof ( data_row ) );

        if ( new_row == NULL ) { return; }

        memset ( new_row , 0 , sizeof ( data_row ) );

        // 设置双向关联
        new_row->parent = node;
        node->row       = new_row;

        // 初始化数据行字段
        new_row->id = node->id;  // 继承节点ID
    }
    // 递归处理子树
    attach_data_to_leaves ( node->left_node );
    attach_data_to_leaves ( node->right_node );
}

//
data_col *
create_data_col ( char * field_name , void * value , size_t value_size ,
                  size_t value_type , bool value_unique ) {

    // 防止总分配溢出:ml-citation{ref="3,7" data="citationList"}
    if ( value_size > SIZE_MAX - sizeof ( data_col ) ) {
        return NULL;
    }

    // 限制单次分配上限(1TB):ml-citation{ref="4" data="citationList"}
    if ( value_size >= ( 1ULL << 40 ) ) {
        return NULL;
    }

    // 预留50%安全余量:ml-citation{ref="3" data="citationList"}
    if ( value_size > SIZE_MAX / 2 ) return NULL;

    // 在函数起始处新增全局约束检查
#define MAX_SINGLE_ALLOC (1ULL << 30) // 1GB安全上限:ml-citation{ref="4" data="citationList"}
    if ( value_size == 0 || value_size > MAX_SINGLE_ALLOC ) {
        return NULL;
    }

    // 对齐计算安全校验
    size_t aligned_size = ( ( value_size + sizeof ( void * ) - 1 ) / sizeof ( void * ) ) * sizeof ( void * );
    if ( aligned_size < value_size ) { // 检测对齐溢出:ml-citation{ref="8" data="citationList"}
        return NULL;
    }

    //
    size_t name_len = strnlen ( field_name , DATA_COL_NAME_MAX_LEN );
    if ( ( name_len <= 0 ) || ( value == NULL ) || ( value_size <= 0 ) || ( value_type <= 0 ) ) {
        return NULL;
    }

    data_col * field = malloc ( sizeof ( data_col ) );

#define STATIC_ASSERT( cond , msg ) typedef char static_assert_##msg[(cond)?1:-1]
    STATIC_ASSERT( offsetof ( data_col , value ) % sizeof ( void * ) == 0 , value_alignment_error );

    if ( field == NULL ) { return NULL; }

    memset ( field , 0 , sizeof ( data_col ) );

    field->name = malloc ( strnlen ( field_name , DATA_COL_NAME_MAX_LEN ) + 1 );
    if ( field->name == NULL ) {
        free ( field );
        return NULL;
    }

    memset ( field->name , 0 , strnlen ( field_name , DATA_COL_NAME_MAX_LEN ) + 1 );

    stpncpy ( field->name , field_name , name_len );

    field->name[ name_len ] = '\0';  // 显式终止

    field->value = calloc ( 1 , aligned_size );

    if ( field->value == NULL && aligned_size > 0 ) {
        free ( field->name );
        free ( field );
        return NULL;
    }

    memset ( field->value , 0 , value_size );

    memcpy ( field->value , value , value_size );

    field->value_size = value_size;

    field->value_type = value_type;

    field->value_unique = value_unique;

    return field;
}

void free_data_col ( data_col * field ) {
    if ( field != NULL ) {
        if ( field->value != NULL ) {
            free ( field->value );
        }
        if ( field->name != NULL ) {
            free ( field->name );
        }
        free ( field );
    }
}

// 创建新数据行
data_row * create_data_row ( size_t row_id , data_col * fields[] , size_t fields_count ,
                             branch_node * parent_node ) {
    if ( row_id <= 0 ) {
        return NULL;
    }
    data_row * row = malloc ( sizeof ( data_row ) );
    if ( row == NULL ) {
        return NULL;
    }
    memset ( row , 0 , sizeof ( data_row ) );
    row->id = row_id;
    unsigned       row_field_index  = 0;
    unsigned       row_fields_count = 0;
    for ( unsigned index            = 0 ; index < fields_count ; index++ ) {
        if ( index >= DATA_COL_MAX_SIZE ) break;
        if ( fields[ index ] != NULL ) {
            fields[ row_field_index ]->row_id = row->id;
            row->cols[ row_field_index ] = create_data_col ( fields[ index ]->name , fields[ index ]->value ,
                                                             fields[ index ]->value_size ,
                                                             fields[ index ]->value_type ,
                                                             fields[ index ]->value_unique );
            row_field_index++;
            row_fields_count++;
        }
    }
    row->fields_count = row_fields_count;
    row->parent       = parent_node;
    return row;
}

//
void free_data_row ( data_row * row ) {
    if ( row != NULL ) {
        for ( size_t index = 0 ; index < row->fields_count ; index++ ) {
            free_data_col ( ( data_col * ) row->cols[ index ] );
        }
        if ( row->next_node != NULL ) {
            if ( row->previous_node != NULL ) {
                row->next_node->previous_node = row->previous_node;
            } else {
                row->next_node->previous_node = NULL;
            }
        }
        if ( row->previous_node != NULL ) {
            if ( row->next_node != NULL ) {
                row->previous_node->next_node = row->next_node;
            } else {
                row->previous_node->next_node = NULL;
            }
        }
        if ( row->parent != NULL ) {
            row->parent->row = NULL;
        }
        free ( row );
    }
}

// 递归检测指定ID的叶子节点是否存在
int exist_data ( branch_node * node , size_t target_id , int depth ) {

    if ( ( node == NULL ) || ( depth > MAX_DEPTH ) ) return 0;

    if ( ( node->left_node == NULL ) && ( node->right_node == NULL ) && ( node->row_id == target_id ) ) {
        return 1;
    }

    if ( node->left_node && target_id <= node->left_node_end ) {
        return exist_data ( node->left_node , target_id , depth + 1 );
    }

    if ( ( node->left_node != NULL ) && target_id <= node->left_node_end ) {
        return exist_data ( node->right_node , target_id , depth + 1 );
    }

    return 0;
}


// 递归插入数据到指定ID的叶子节点
int set_data ( branch_node * node ,
               size_t target_id ,
               data_col * fields[] ,
               size_t fields_count ,
               int depth ,
               lock_manager_t * mgr ) {

    //当前节点为空，或者搜索深度已经达到上限
    if ( ( node == NULL ) || ( depth > MAX_DEPTH ) ) {
        return 0;
    }

    //当前正在被查询的节点
    printf ( "\nnode->id : %llu , node->row_id : %llu , node->left_node : %p , node->right_node : %p \n" , (
            unsigned long long ) node->id , (
                     unsigned long long ) node->row_id , node->left_node , node->right_node );


    if ( ( node->left_node == NULL ) && ( node->right_node == NULL ) && ( node->row_id == target_id ) ) {

        //当前节点不存在子节点
        printf ( "\nnode->id : %llu , node->row_id : %llu , The current node does not have any child nodes\n" , (
                unsigned long long ) node->id , (
                         unsigned long long ) node->row_id );

        //找到匹配节点
        printf ( "\nnode->id : %llu , node->row_id : %llu , Matching nodes have been found\n" , (
                unsigned long long ) node->id , (
                         unsigned long long ) node->row_id );

        //加锁
        acquire_lock ( mgr , target_id );
        //写入索引信息
        for ( unsigned fields_index = 0 ; fields_index < fields_count ; fields_index++ ) {
            if (( fields[ fields_index ]->value_type == DATA_COL_VALUE_TYPE_STRING )&&(fields[ fields_index ]->value_size<=MAX_INDEX_KEY_LEN)) {
                //创建MD5值
                char * md5_value = md5 (fields[ fields_index ]->value);
                if( md5_value == NULL ){
                    release_lock ( mgr , target_id );
                    return 0;
                }
                //当前字段的内容具备唯一特性
                if ( fields[ fields_index ]->value_unique == true ) {
                    //尝试写入唯一索引
                    int ret = insert_unique_key ( node->table->unique_managers[ fields_index ] ,
                                                  md5_value ,
                                                  target_id );
                    //释放MD5值
                    free(md5_value);
                    //
                    //唯一索引写入失败（唯一索引已存在）
                    if ( ret == -1 ) {
                        release_lock ( mgr , target_id );
                        return 0;
                    }
                //当前字段的内容不具备唯一特性
                } else {
                    //
                    if(node->table->unique_index_managers[ fields_index ]==NULL){
                        node->table->unique_index_managers[ fields_index ]=create_unique_index_manager (fields[ fields_index ]->name);
                    }
                    //尝试写入普通索引
                    bool ret = insert_unique_index_key_for_row_id ( node->table->unique_index_managers[ fields_index ] , md5_value , target_id );
                    //释放MD5值
                    free(md5_value);
                    //普通索引写入失败
                    if(ret!=true){
                        release_lock ( mgr , target_id );
                        return 0;
                    }

                }
            }
        }
        //当前节点已存在数据
        if ( node->row ) {
            printf ( "\nWarning: Overwriting data at Row Id : %llu\n" ,
                     ( unsigned long long ) target_id );
            free_data_row ( node->row );
        }
        //为当前节点赋予数据
        node->row = create_data_row ( target_id , fields , fields_count , node );
        //解锁
        release_lock ( mgr , target_id );

        //检查当前节点的数据是否更新成功
        if ( node->row != NULL ) {
            //当前节点的数据更新成功
            printf (
                    "\nnode->id : %llu , node->row_id : %llu , The data update of the current node is successful\n" ,
                    (
                            unsigned long long ) node->id , (
                            unsigned long long ) node->row_id );
        } else {
            //当前节点的数据更新失败
            printf ( "\nnode->id : %llu , node->row_id : %llu , The data update of the current node has failed\n" ,
                     (
                             unsigned long long ) node->id , (
                             unsigned long long ) node->row_id );
        }

        return ( ( node->row != NULL ) ? 1 : 0 );
    }

    //
    if ( ( node->left_node != NULL ) && target_id <= node->left_node_end ) {

        //目标节点小于等于当前节点的左侧节点
        printf (
                "\nnode->left_node->id : %llu , node->left_node->row_id : %llu ， target_id : %llu , node->left_node_end : %llu , The target node is less than or equal to the maximum addressable range of the left node, and addressing starts from the left node\n" ,
                (
                        unsigned long long ) node->left_node->id , (
                        unsigned long long ) node->left_node->row_id , (
                        unsigned long long ) target_id , (
                        unsigned long long ) node->left_node_end );

        return set_data ( node->left_node , target_id , fields ,
                          fields_count , depth + 1 , mgr );
    }

    if ( ( node->right_node != NULL ) && target_id > node->left_node_end ) {

        //目标节点大于当前节点的左侧节点
        printf (
                "\nnode->left_node->id : %llu , node->left_node->row_id : %llu ， target_id : %llu , node->left_node_end : %llu , The target node is larger than the maximum addressable range of the left node, and addressing starts from the right node\n" ,
                (
                        unsigned long long ) node->left_node->id , (
                        unsigned long long ) node->left_node->row_id , (
                        unsigned long long ) target_id , (
                        unsigned long long ) node->left_node_end );

        return set_data ( node->right_node , target_id , fields ,
                          fields_count , depth + 1 , mgr );
    }

    return 0;
}

// 递归获得指定ID的叶子节点
branch_node * get_data ( branch_node * node , size_t target_id , int depth ) {

    if ( ( node == NULL ) || ( depth > MAX_DEPTH ) ) return NULL;

    if ( ( node->left_node == NULL ) && ( node->right_node == NULL ) && ( node->row_id == target_id ) &&
         ( node->row != NULL ) ) {
        return node;
    }

    if ( ( node->left_node != NULL ) && ( target_id <= node->left_node_end ) ) {
        return get_data ( node->left_node , target_id , depth + 1 );
    }

    if ( ( node->right_node != NULL ) && ( target_id > node->left_node_end ) ) {
        return get_data ( node->right_node , target_id , depth + 1 );
    }

    return NULL;
}

// 递归删除指定ID的叶子节点
int delete_data ( branch_node * node , size_t target_id , int depth , lock_manager_t * mgr ) {

    if ( ( node == NULL ) || ( depth > MAX_DEPTH ) ) return 0;

    if ( ( node->left_node == NULL ) && ( node->right_node == NULL ) && ( node->row_id == target_id ) ) {
        if ( node->row != NULL ) {
            //
            acquire_lock ( mgr , target_id );
            //
            printf ( "\nDeleting data at ID %llu\n" ,
                     ( unsigned long long ) target_id );
            //
            for ( unsigned fields_index = 0 ; fields_index < DATA_COL_MAX_SIZE ; fields_index++ ) {
                if((node->row->cols[fields_index]->value_type==DATA_COL_VALUE_TYPE_STRING)&&(node->row->cols[fields_index]->value_size<=MAX_KEY_LEN)){
                    //
                    char * md5_value = md5(node->row->cols[fields_index]->value);
                    //
                    delete_unique_key (node->table->unique_managers[fields_index],md5_value);
                    //
                    if(node->table->unique_index_managers[fields_index]!=NULL){
                        //
                       delete_unique_index_key_by_row_id (node->table->unique_index_managers[fields_index],md5_value,node->row->id);
                    }
                    //
                    free(md5_value);
                    //
                }
            }
            //
            free_data_row ( node->row );
            node->row = NULL;  // 清除悬垂指针
            //
            release_lock ( mgr , target_id );
            //
        }
        return 1;
    }

    if ( ( node->left_node != NULL ) && ( target_id <= node->left_node_end ) ) {
        return delete_data ( node->left_node , target_id , depth + 1 , mgr );
    }

    if ( ( node->right_node != NULL ) && ( target_id > node->left_node_end ) ) {
        return delete_data ( node->right_node , target_id , depth + 1 , mgr );
    }

    return 0;
}

//
void print_branch_node_tree ( branch_node * node , int depth ) {
    if ( node == NULL ) return;

    // 添加层级缩进
    for ( int i = 0 ; i < depth ; i++ ) printf ( "  " );

    printf ( "NODE %llu [L:%llu-%llu R:%llu-%llu] ROW:%llu\n" ,
             ( unsigned long long ) node->id ,
             ( unsigned long long ) node->left_node_start ,
             ( unsigned long long ) node->left_node_end ,
             ( unsigned long long ) node->right_node_start ,
             ( unsigned long long ) node->right_node_end ,
             ( unsigned long long ) node->row_id );

    print_branch_node_tree ( node->left_node , depth + 1 );
    print_branch_node_tree ( node->right_node , depth + 1 );
}

//
void print_rows ( branch_node * node , int * count ) {
    if ( node == NULL ) return;

    print_rows ( node->left_node , count );

    if ( node->row_id > 0 ) {
        if ( * count > 0 ) printf ( "\n" );  // 非首项才加空行
        printf ( "ROW ID  : %-8llu\nNODE ID : %-8llu\nRANGE   : [%llu-%llu | %llu-%llu]\n" ,
                 ( unsigned long long ) node->row_id ,
                 ( unsigned long long ) node->id ,
                 ( unsigned long long ) node->left_node_start ,
                 ( unsigned long long ) node->left_node_end ,
                 ( unsigned long long ) node->right_node_start ,
                 ( unsigned long long ) node->right_node_end );
        ( * count )++;
    }

    print_rows ( node->right_node , count );
}

//
void print_row ( const branch_node * node ) {

    if ( node == NULL || node->row_id <= 0 ) return;

    printf ( "\n"
             "ROW ID            :  %-10llu\n"
             "NODE ID           :  %-10llu\n"
             "LEFT RANGE        :  %llu~%llu\n"
             "RIGHT RANGE       :  %llu~%llu\n"
             "PARENT NODE ID    :  %llu\n"
             "PREVIOUS NODE ID  :  %llu\n"
             "NEXT NODE ID      :  %llu\n"
             "FIELDS COUNT      :  %llu\n" ,
             ( unsigned long long ) node->row_id ,
             ( unsigned long long ) node->id ,
             ( unsigned long long ) node->left_node_start ,
             ( unsigned long long ) node->left_node_end ,
             ( unsigned long long ) node->right_node_start ,
             ( unsigned long long ) node->right_node_end ,
             ( unsigned long long ) ( ( node->row != NULL ) ? ( ( node->row->parent != NULL ) ? node->row->parent->id
                                                                                              : 0 ) : 0 ) ,
             ( unsigned long long ) ( ( node->row != NULL ) ? ( ( node->row->previous_node != NULL )
                                                                ? node->row->previous_node->id : 0 ) : 0 ) ,
             ( unsigned long long ) ( ( node->row != NULL ) ? ( ( node->row->next_node != NULL )
                                                                ? node->row->next_node->id : 0 ) : 0 ) ,
             ( unsigned long long ) ( ( node->row != NULL ) ? node->row->fields_count : 0 ) );
    printf ( "FIELDS            :  " );
    if ( node->row != NULL ) {
        for ( unsigned index = 0 ; index < node->row->fields_count ; index++ ) {
            if ( node->row->cols[ index ] != NULL ) {
                printf ( "name ( %s ) " , node->row->cols[ index ]->name );
                printf ( "type ( %llu ) " , ( unsigned long long ) node->row->cols[ index ]->value_type );
                printf ( "value ( " );
                //
                for ( size_t data_index = 0 ;
                      data_index < node->row->cols[ index ]->value_size ; data_index++ ) {
                    printf ( "\\x%02x" ,
                             ( unsigned char ) ( ( char * ) node->row->cols[ index ]->value )[ data_index ] );
                }
                //
                printf ( " ) " );
                //
                if ( index < ( node->row->fields_count - 1 ) ) {
                    printf ( " , " );
                }
            }
        }
    }
    printf ( "\n" );
}

// 数据管理器实现

//创建数据管理器（实例）
data_manager * create_data_manager ( ) {
    return calloc ( 1 , sizeof ( data_manager ) );
}

void free_data_manager ( data_manager * d_m ) {
    if ( d_m != NULL ) {
        free ( d_m );
    }
}

//数据服务器实现

//创建数据库服务器
data_server * create_data_server ( size_t server_id , const char * key , size_t ipv4 , size_t port ) {
    data_server * server = calloc ( 1 , sizeof ( data_server ) );
    if ( !server ) return NULL;

    server->id   = server_id;
    server->key  = strdup ( key );
    server->ipv4 = ipv4;
    server->port = port;
    return server;
}

//向数据管理器中添加数据库服务器
int add_data_server ( data_manager * mgr , data_server * server ) {
    if ( !mgr || !server || server->id >= DATA_SERVER_MAX_SIZE ) return -1;
    if ( mgr->data_servers[ server->id ] ) return -2; // ID冲突

    mgr->data_servers[ server->id ] = server;
    return 0;
}

//在数据管理器中查找数据库服务器
data_server * find_data_server ( data_manager * mgr , size_t server_id ) {
    if ( !mgr || server_id >= DATA_SERVER_MAX_SIZE ) return NULL;
    return mgr->data_servers[ server_id ];
}

//从数据管理器中移除数据库服务器
int remove_data_server ( data_manager * mgr , size_t server_id ) {
    if ( !mgr || server_id >= DATA_SERVER_MAX_SIZE ) return -1;
    if ( !mgr->data_servers[ server_id ] ) return -2;
    for ( unsigned db_index = 0 ; db_index < DATA_BASE_MAX_SIZE ; db_index++ ) {
        if ( mgr->data_servers[ server_id ]->data_bases[ db_index ] != NULL ) {
            return -3;
        }
    }
    free ( mgr->data_servers[ server_id ] );
    mgr->data_servers[ server_id ] = NULL;
    return 0;
}

//数据库实现

// 创建数据库
data_base * create_data_base ( size_t db_id , const char * key ) {
    data_base * db = calloc ( 1 , sizeof ( data_base ) );
    if ( !db ) return NULL;

    db->id  = db_id;
    db->key = strdup ( key );
    return db;
}

//向数据库服务器中添加数据库
int add_data_base ( data_server * server , data_base * db ) {
    if ( !server || !db || db->id >= DATA_BASE_MAX_SIZE ) return -1;
    if ( server->data_bases[ db->id ] ) return -2;

    server->data_bases[ db->id ] = db;
    return 0;
}

//在数据管理器中查找数据库服务器
data_base * find_data_base ( data_server * server , size_t db_id ) {
    if ( !server || db_id >= DATA_BASE_MAX_SIZE ) return NULL;
    return server->data_bases[ db_id ];
}


//从数据库服务器中移除数据库
int remove_data_base ( data_server * server , size_t db_id ) {
    if ( !server || db_id >= DATA_BASE_MAX_SIZE ) return -1;
    if ( !server->data_bases[ db_id ] ) return -2;
    for ( unsigned tb_index = 0 ; tb_index < DATA_TABLE_MAX_SIZE ; tb_index++ ) {
        if ( server->data_bases[ db_id ]->data_tables[ tb_index ] != NULL ) {
            return -3;
        }
    }
    free ( server->data_bases[ db_id ] );
    server->data_bases[ db_id ] = NULL;
    return 0;
}

//数据表实现

//创建数据表
data_table *
create_data_table ( size_t tb_id , const char * key , char * fd_names[] , unsigned fds_count ) {
    data_table * table = calloc ( 1 , sizeof ( data_table ) );
    if ( !table ) return NULL;

    table->id  = tb_id;
    table->key = strdup ( key );
    if ( fds_count > DATA_COL_MAX_SIZE ) { fds_count = DATA_COL_MAX_SIZE; }
    for ( unsigned fds_index = 0 ; fds_index < fds_count ; fds_index++ ) {
        table->unique_managers[ fds_index ] = create_unique_manager ( fd_names[ fds_index ] );
    }
    return table;
}

//绑定根数据节点到数据表
data_table * bind_data_table_root_branch_node ( data_table * table , branch_node * root_branch_node ) {
    if ( table == NULL ) {
        return NULL;
    }
    if ( root_branch_node == NULL ) {
        return NULL;
    }
    table->root_branch_node = root_branch_node;
    return table;
}

//向数据库中添加数据表
int add_data_table ( data_base * db , data_table * table ) {
    if ( !db || !table || table->id >= DATA_TABLE_MAX_SIZE ) return -1;
    if ( db->data_tables[ table->id ] ) return -2;

    db->data_tables[ table->id ] = table;
    return 0;
}

//在数据库中查找数据表
data_table * find_data_table ( data_base * db , size_t tb_id ) {
    if ( !db || tb_id >= DATA_TABLE_MAX_SIZE ) return NULL;
    return db->data_tables[ tb_id ];
}


//从数据库中移除数据表
int remove_data_table ( data_base * db , size_t tb_id ) {
    if ( !db || tb_id >= DATA_TABLE_MAX_SIZE ) return -1;
    if ( !db->data_tables[ tb_id ] ) return -2;
    if ( db->data_tables[ tb_id ]->root_branch_node != NULL ) {
        return -3;
    }
    free ( db->data_tables[ tb_id ] );
    db->data_tables[ tb_id ] = NULL;
    return 0;
}
