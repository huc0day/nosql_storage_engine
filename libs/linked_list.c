#include "linked_list.h"

// 静态变量：线程安全的自增ID
static unsigned long long linked_next_id = 1;           // 初始值为1
static pthread_mutex_t linked_id_mutex = PTHREAD_MUTEX_INITIALIZER;  // 静态初始化互斥锁
// 静态变量：线程安全的自增ID
static unsigned long long pages_next_id = 1;           // 初始值为1
static pthread_mutex_t pages_id_mutex = PTHREAD_MUTEX_INITIALIZER;  // 静态初始化互斥锁
//静态变量：线程安全的操作锁
static pthread_mutex_t linked_id_operate_mutex = PTHREAD_MUTEX_INITIALIZER;  // 静态初始化互斥锁

// 生成自增ID的函数（线程安全）
unsigned long long generate_threadsafe_linked_id ( ) {
    unsigned long long current_id;

    // 加锁保护共享变量
    pthread_mutex_lock(&linked_id_mutex);
    current_id = linked_next_id++;
    pthread_mutex_unlock(&linked_id_mutex);

    return current_id;
}

// 生成自增ID的函数（线程安全）
unsigned long long generate_threadsafe_pages_id ( ) {
    unsigned long long current_id;

    // 加锁保护共享变量
    pthread_mutex_lock(&pages_id_mutex);
    current_id = pages_next_id++;
    pthread_mutex_unlock(&pages_id_mutex);

    return current_id;
}



//私有方法：备注：以_tmp_命名开头的方法不可在头文件外部调用
//
static int _tmp_newDataMem (
        LINKED_LIST_POINT this_point ,
        COMMON_MEMORY data ,
        SIZE size
                    ) {
    if (!this_point || !data || size == 0) {
        return -1;  // 错误提示：参数错误
    }
    this_point -> data = ( COMMON_MEMORY ) malloc ( size );  //开辟新值的存储空间
    memset ( this_point -> data , 0 , size );             //初始化新值存储空间
    memcpy ( this_point -> data , data , size );          //内存数据拷贝
    this_point -> size = size;                            //内存大小
    return 0;
}

//
static int _tmp_oldDataMem (
        LINKED_LIST_POINT this_point ,
        COMMON_MEMORY data ,
        SIZE size
                    ) {
    if (!this_point || !data || size == 0) {
        return -1;  // 错误提示：参数错误
    }
    memset ( this_point -> data , 0 , this_point -> size );   //重新初始化旧值存储空间
    memcpy ( this_point -> data , data , size );              //内存数据拷贝
    this_point -> size = size;                                //内存大小
    return 0;
}

//向链表中新增一条记录（私有方法，不可在外部调用）
static LINKED_LIST_POINT _insertLinkedList (
        LINKED_LIST_POINT parent_point , //父节点指针
        LINKED_LIST_POINT last_point ,   //上一条记录的指针
        INDEX id ,                        //当前记录的ID数值
        STRING name ,                    //当前记录的KEY
        COMMON_MEMORY data ,             //当前记录的值
        SIZE size                        //当前记录的值大小
                                    ) {

    //参数合法性校验
    if (!name || !data || size == 0) {
        return NULL;  // 无效输入，返回NULL
    }

    //创建当前记录的存储空间
    LINKED_LIST_POINT this_point = ( LINKED_LIST_POINT ) malloc ( LINKED_LIST_LEN );
    if (!this_point) {
        return NULL;  // 内存分配失败
    }

    //初始化当前记录的存储空间
    memset ( this_point , 0 , LINKED_LIST_LEN );

    //深拷贝name
    SIZE name_len = strlen(name) + 1;
    this_point->name = (STRING)malloc(name_len);
    if (!this_point->name) {
        free(this_point);  // 释放节点内存
        return NULL;
    }
    memset ( this_point->name , 0 , name_len );
    memcpy(this_point->name, name, name_len);              //当前记录KEY

    //深拷贝data
    this_point->data = (COMMON_MEMORY)malloc(size);
    if (!this_point->data) {
        free(this_point->name);
        free(this_point);
        return NULL;
    }
    memset ( this_point->data , 0 , size );
    memcpy(this_point->data, data, size);                       //当前记录值
    this_point->size = size;                                    //当前记录值大小

    //为当前记录赋值
    this_point -> parent_point     = parent_point;              //父节点指针
    this_point -> child_head_point = NULL;                      //子链表的头节点指针
    this_point -> last_point       = last_point;                //上一条记录的指针
    this_point -> next_point       = NULL;                      //下一条记录的指针，初始化为 NULL
    this_point -> id               = id;                        //当前记录ID

    if ( parent_point != NULL ) {                               //当前节点存在父节点
        if ( parent_point -> child_head_point == NULL ) {       //当前父节点还不存在任何子节点
            parent_point -> child_head_point = this_point;      //将当前节点设置为父节点的子链表头节点
        }
    }
    //如果当前记录不是第一条记录
    if ( last_point != NULL ) { //当前元素不是头元素
        last_point -> next_point = this_point; //将当前记录的指针赋予给上一条记录
    }

    return this_point; //返回当前记录的指针
}

//更新链表中指定记录的值
ID updateLinkedList (
        LINKED_LIST_POINT this_point ,
        COMMON_MEMORY data ,
        SIZE size
                    ) {
    //
    pthread_mutex_lock(&linked_id_operate_mutex);
    //
    if ( this_point -> data != NULL ) { //如果当前记录存在有效值
        if ( size <= this_point -> size ) { //新值所需的存储空间比旧值小
            //复用之前的存储空间，不再重新开辟
            _tmp_oldDataMem ( this_point , data , size ); //重新初始化旧值的存储空间，并进行内存拷贝
            //printf("update old memory \n");

        } else { //新值所需的存储空间比旧值大
            free ( this_point -> data ); //释放旧值的存储空间
            _tmp_newDataMem ( this_point , data , size ); //生成新值的存储空间，并进行初始化和内存拷贝
            //printf("update new memory\n");

        }
    } else { //当前记录没有有效值（目前DATA指向的是空值）
        _tmp_newDataMem ( this_point , data , size ); //生成新值的存储空间，并进行初始化和内存拷贝
        //printf("update new memory\n");
    }
    //
    pthread_mutex_unlock(&linked_id_operate_mutex);
    //
    return this_point -> id;
}

//删除链表中的记录
int deleteLinkedList (
        LINKED_LIST_POINT this_point
                     ) {
    //
    pthread_mutex_lock(&linked_id_operate_mutex);
    //
    if ( this_point -> child_head_point != NULL ) { //当前节点存在子节点，不能删除；如需删除，需先确保其无任何子节点存在
        pthread_mutex_unlock(&linked_id_operate_mutex);
        return 1; //删除当前节点失败
    }
    if ( this_point -> last_point != NULL ) { //当前节点不是头节点
        this_point -> last_point -> next_point = this_point -> next_point; //将当前节点的上一节点与当前节点的下一节点建立关联
    }
    if ( this_point -> next_point != NULL ) { //当前节点不是尾节点
        this_point -> next_point -> last_point = this_point -> last_point; //将当前节点的下一节点与当前节点的上一节点建立关联
    }
    if ( this_point -> parent_point != NULL ) { //当前节点存在父节点

        if ( this_point -> parent_point -> child_head_point == this_point ) { //当前节点是父节点的子链表头节点
            if ( this_point -> next_point != NULL ) { //当前节点不是尾节点
                if ( this_point -> next_point -> parent_point == this_point -> parent_point ) { //当前节点的下一节点与当前节点同属于一个父节点
                    this_point -> parent_point -> child_head_point = this_point -> next_point; //将当前节点的下一节点设置为父节点的子链表头节点
                } else { //当前节点的下一节点与当前节点并不同属于一个父节点
                    this_point -> parent_point -> child_head_point = NULL; //将父节点的子链表头节点指针设置为NULL
                }

            } else { //当前节点是尾节点
                this_point -> parent_point -> child_head_point = NULL; //将父节点的子链表头节点指针设置为NULL
            }
        }
    }

    free ( this_point -> name ); //删除当前节点的KEY信息
    free ( this_point -> data ); //删除当前节点的DATA信息
    free ( this_point );       //删除当前节点
    //
    pthread_mutex_unlock(&linked_id_operate_mutex);
    //
    return 0; //成功删除当前节点
}

//根据ID查找记录（正序查找）
LINKED_LIST_POINT findByIdInASC (
        LINKED_LIST_POINT head_point ,
        ID id
                                ) {
    //
    pthread_mutex_lock(&linked_id_operate_mutex);
    //
    LINKED_LIST_POINT this_point = head_point;
    while ( this_point != NULL ) {
        if ( this_point -> id == id ) {
            pthread_mutex_unlock(&linked_id_operate_mutex);
            return this_point;
        }
        this_point = this_point -> next_point;
    }
    //
    pthread_mutex_unlock(&linked_id_operate_mutex);
    //
    return NULL;
}

//根据ID查找记录（倒序查找）
LINKED_LIST_POINT findByIdInDESC (
        LINKED_LIST_POINT foot_point ,
        ID id
                                 ) {
    //
    pthread_mutex_lock(&linked_id_operate_mutex);
    //
    LINKED_LIST_POINT this_point = foot_point;
    while ( this_point != NULL ) {
        if ( this_point -> id == id ) {
            pthread_mutex_unlock(&linked_id_operate_mutex);
            return this_point;
        }
        this_point = this_point -> last_point;
    }
    //
    pthread_mutex_unlock(&linked_id_operate_mutex);
    //
    return NULL;
}

char * getName ( char * name ) {
    size_t name_length   = strlen ( name );
    size_t name_mem_size = ( name_length + 1 ); //'\0'占1字节
    char * name_mem = ( char * ) malloc ( name_mem_size );
    memset ( name_mem , 0 , name_mem_size );
    memcpy ( name_mem , name , name_length );
    return name_mem;
}

char * getStringData ( char * data ) {
    size_t data_length   = strlen ( data );
    size_t data_mem_size = ( data_length + 1 ); //'\0'占1字节
    char * data_mem = ( char * ) malloc ( data_mem_size );
    memset ( data_mem , 0 , data_mem_size );
    memcpy ( data_mem , data , data_length );
    return data_mem;
}

//新增数据
LINKED_LIST_POINT
insertData ( char * t_name , void * data , unsigned long long data_size , LINKED_LIST_POINT * page_head_point ) {
    //
    pthread_mutex_lock(&linked_id_operate_mutex);
    //
    char * name = getName ( t_name );                                              //取缔用数组方式传递KEY
    static LINKED_LIST_POINT page_point      = NULL;                               //当前页指针
    static LINKED_LIST_POINT last_page_point = NULL;                               //上一页指针
    static LINKED_LIST_POINT last_point      = NULL;                               //上一条记录的指针
    INDEX                    id              = generate_threadsafe_linked_id();    //ID计数器
    int                      page_size       = LINKED_LIST_PAGE_SIZE;              //分页大小
    char * page_name = NULL;                                                       //初始化页名称

    //新增一条记录
    LINKED_LIST_POINT new_point = _insertLinkedList (
            page_point ,      //父指针
            last_point ,      //上一条记录的指针，对于第一条记录来说，其值为NULL
            id ,               //记录的ID数值
            name ,            //记录的KEY，用于索引
            data ,            //记录的DATA，并不一定是字符串类型，但是目前以字符串类型作为示例
            data_size         //记录的DATA所需占用的内存空间的大小
                                                    );

    //生成当前分页
    if ( ( id == 1 ) || ( ( id >= page_size ) && ( ( id % page_size ) == 0 ) ) ) {

        ID   page_id = generate_threadsafe_pages_id();                                                //页ID
        char page_tmp_name[LINKED_LIST_PAGE_NAME_SIZE];                                   //页临时名称
        snprintf ( page_tmp_name , LINKED_LIST_PAGE_NAME_SIZE , "page_%012d" , page_id ); //页临时名称
        page_name  = getName ( page_tmp_name );                                           //设置页名称
        //当前页
        page_point = ( LINKED_LIST_POINT ) malloc ( LINKED_LIST_LEN );   //创建当前节点
        page_point -> parent_point     = NULL;                           //父节点指针
        page_point -> child_head_point = new_point;                      //当前页中头元素的指针
        page_point -> last_point       = last_page_point;                //上一页记录的指针
        page_point -> next_point       = NULL;                           //下一页的指针
        page_point -> id               = page_id;                        //页ID
        page_point -> name             = page_name;                      //页名称
        page_point -> data             = NULL;                           //记录数据
        page_point -> size             = 0;                              //记录数据大小

        last_page_point = page_point;                              //将上一页指针设置为当前页指针

        if ( page_id == 1 ) { //第一页
            ( * page_head_point ) = page_point;                       //设置分页头指针
        }

        if ( page_point -> last_point != NULL ) {                      //当前页不是第一页
            page_point -> last_point -> next_point = page_point;       //建立当前页与上一页的关联
        }
    }

    new_point -> parent_point = page_point; //设置新记录的父节点指针
    last_point = new_point;                 //重置下次调用时上一条记录的指针指向当前记录
    //
    pthread_mutex_unlock(&linked_id_operate_mutex);
    //
    return new_point;   //返回新创建的记录的指针
}

//根据ID采取分页方式查找记录
LINKED_LIST_POINT findById ( ID id , LINKED_LIST_POINT page_head_point ) {
    //
    pthread_mutex_lock(&linked_id_operate_mutex);
    //
    INDEX index = 1 , page;
    if ( id < 0 ) {
        pthread_mutex_unlock(&linked_id_operate_mutex);
        return NULL;
    }
    if ( id < LINKED_LIST_PAGE_SIZE ) {
        page = 1;
    } else {
        page = ( ( id % LINKED_LIST_PAGE_SIZE ) == 0 ) ? ( id / LINKED_LIST_PAGE_SIZE ) : (
                ( id / LINKED_LIST_PAGE_SIZE ) + 1 );
    }
    LINKED_LIST_POINT page_point = page_head_point;
    while ( page_point != NULL ) {

        if ( index == page ) {
            LINKED_LIST_POINT point = page_point -> child_head_point;
            while ( point != NULL ) {
                if ( point -> id == id ) {
                    pthread_mutex_unlock(&linked_id_operate_mutex);
                    return point;
                }
                point = point -> next_point;
            }
        }

        page_point = page_point -> next_point;

        index ++;
    }
    //
    pthread_mutex_unlock(&linked_id_operate_mutex);
    //
    return NULL;
}

