# nosql_storage_engine
C language project (NOSQL storage engine)

以下是该项目《nosql_storage_engine》的详细技术总结：
 

一、项目概述

这是一个基于C语言开发的NoSQL存储引擎原型系统，采用分层架构设计，包含四大核心模块：

哈希表模块（Hash Manager）

唯一键管理模块（Unique Manager）

多级索引模块（Index Manager）

并发控制模块（Lock Manager）

线程安全链表（Linked List）

数据库管理系统（DB Struct Data）

 

二、核心模块详解

1. 哈希表模块（hash_manager）

设计目标：高效键值存储，支持两种冲突解决策略

两种实现：

UniqueEntry表（开放寻址法）

键类型： char* （字符串）

值类型： int 

冲突解决：线性探测（开放寻址法）

删除策略：惰性删除（标记 is_deleted 位）

哈希函数：DJB2算法（初始值5381，乘数33）

表大小：1048577（2^20 +1，质数优化冲突）

IndexEntry表（链地址法）

键类型： uint64_t （整数）

值类型： void* （泛型数据）

冲突解决：链表存储冲突元素（头插法）

内存管理： strdup 分配键内存，避免内存泄漏

接口功能：

 insert()  /  query()  /  delete() （通用CRUD）

支持错误码（如 KEY_NOT_FOUND ）

技术亮点：

大质数表大小降低哈希冲突概率

字符串内存安全（ strdup +手动释放）

惰性删除优化写入性能

 

2. 唯一键管理模块（unique_manager）

数据结构：Trie树（前缀树）

节点设计：

 unique_key_node ：37个子节点（a-z0-9+_字符集）

 row_id 存储行标识

 level 记录节点深度

核心功能：

 insert_unique_key() ：O(L)时间插入键值对

 search_unique_key() ：精确查找行ID

 delete_unique_key() ：惰性删除（标记清除）

 free_unique_manager() ：递归释放树内存

应用场景：

代理服务器路由表（前缀匹配）

关键词过滤系统

优势：

天然支持前缀搜索

无哈希冲突

键有序存储

 

3. 多级索引模块（index_manager）

数据结构：两级树结构

第一级：37叉树（a-z0-9+_字符集）

第二级：10叉树（0-9数字）

核心功能：

 insert_unique_index_key() ：构建复合索引

 get_row_ids_paginated() ：分页查询行ID

 traverse_keys_helper() ：递归遍历所有键

 callback 机制处理查询结果

技术特性：

行ID级联操作（插入/删除）

动态内存管理（避免内存泄漏）

多级指针操作保障树结构完整性

适用场景：

高效范围查询

分布式数据库路由

 

4. 并发控制模块（lock_manager）

设计目标：细粒度行级锁管理

数据结构：

 row_lock_t ：互斥锁+条件变量+引用计数

 lock_bucket_t ：哈希桶（开放寻址法解决冲突）

 pthread_rwlock_t ：外层读写锁优化并发

关键机制：

锁重入：通过 refcount 支持同一线程多次加锁

锁升级：阻塞等待与信号量唤醒

安全边界： REFCOUNT_MAX 防止计数溢出

应用场景：

高并发事务处理

实时数据更新场景

 

5. 线程安全链表（linked_list）

数据结构：双向链表+父子指针

线程安全：

三个静态互斥锁（ linked_id_mutex / pages_id_mutex / operate_mutex ）

原子ID生成函数（ generate_threadsafe_*_id ）

功能特性：

正序/倒序查找（分页支持）

子链表实现树形结构

动态内存管理（ LINKED_LIST_PAGE_SIZE 控制粒度）

适用场景：

内存数据库索引

任务调度队列

 

6. 数据库管理系统（db_struct_data）

架构设计：四级分层模型

数据管理器 → 服务器 → 数据库 → 数据表

核心组件：

数据行：双向链表存储，支持三种数据类型（数值/字符串/二进制）

二叉树索引：优化数据范围查询

根节点动态扩容（两倍增长策略）

行ID范围存储（ _start / _end ）

并发控制：集成 lock_manager 实现行级锁

技术特性：

跨平台内存对齐（ ALIGNED_ALLOC 宏）

加密模块集成（ hybrid.h ）

字段名长度限制（ DATA_COL_NAME_MAX_LEN ）

局限性：

二叉树深度过大可能导致递归性能下降（建议最大深度≤20）

 

三、项目优势

模块化设计：各模块独立，可灵活组合

高性能：

哈希表（O(1)平均复杂度）

Trie树（O(L)前缀匹配）

二叉树索引（高效范围查询）

线程安全：多层级锁机制保障并发安全

内存管理：严格的内存分配与释放策略

扩展性：支持自定义数据类型和回调函数

 

四、待优化方向

二叉树深度优化：动态平衡树结构（如AVL/红黑树）

哈希表负载因子：动态扩容策略

锁粒度细化：减少锁竞争（如分段锁）

功能完善：

支持事务（ACID特性）

完善持久化存储

增加网络通信模块

 

五、适用场景

内存数据库：高频读写场景（如实时统计）

缓存系统：高并发键值存储（如代理路由表）

嵌入式系统：轻量级NoSQL解决方案

教学演示：数据结构与并发编程实践案例

 

六、技术栈

编程语言：C99标准

并发原语：POSIX线程库（pthread）

内存管理： malloc / calloc / realloc / free 

加密算法：混合加密（ hybrid.h ）

测试框架：单元测试（如 *_test.c 文件）

 

七、项目状态

当前阶段：Alpha版（优化与测试中）

后续计划：

增强功能（如持久化、事务）

性能调优（压力测试）

文档完善（API手册+架构图）

 

该项目的核心价值在于通过多种数据结构与并发控制技术的组合，提供高性能、可扩展的NoSQL存储解决方案，尤其适合需要高吞吐量和低延迟的实时数据处理场景。


源码内容介绍

hash_manager.h

这是一个C语言实现的哈希表头文件，定义了两种哈希表结构：

    UniqueEntry表特点：

    使用开放寻址法处理冲突
    键类型为char*，值类型为int
    表大小为1048577(2^20+1，选择质数减少冲突)
    支持惰性删除(is_deleted标记)

    IndexEntry表特点：

    使用链地址法处理冲突
    键类型为uint64_t，值类型为void*
    每个桶使用链表存储冲突元素

提供的接口包括：

    两种键类型的哈希计算函数
    插入/查询/删除操作
    两种表结构各自的操作函数

这个头文件设计合理，考虑了：

    冲突处理(开放寻址法和链地址法)
    惰性删除优化
    通用数据存储(void*)
    错误处理(KEY_NOT_FOUND)
    大质数表大小选择

需要配合对应的.c实现文件使用（hash_manager.c），实现这些接口的具体功能。

hash_manager.c

这是一个C语言实现的哈希表C源文件,代码实现了一个高效的哈希表系统，包含两种不同的哈希表实现方式：

    ‌唯一索引哈希表(UniqueEntry表)‌：

    使用开放寻址法处理冲突，采用线性探测策略
    实现了惰性删除机制(is_deleted标记位)
    使用DJB2哈希算法(初始值5381，乘数33)计算字符串键的哈希值
    提供完整的CRUD操作接口(insert/query/delete)

    ‌普通索引哈希表(IndexEntry表)‌：

    使用链地址法处理冲突
    直接对uint64_t键取模作为哈希值
    采用头插法维护链表结构
    支持任意数据类型存储(void*指针)

代码中的几个关键实现细节：

    哈希表大小TABLE_SIZE选择为质数(1048577)，减少哈希冲突
    字符串键使用strdup分配内存，确保键值独立性
    删除操作正确处理了内存释放问题
    使用do-while循环确保完整遍历整个哈希表

技术总结：该实现综合了两种主流的哈希冲突解决方法，适用于不同场景需求。开放寻址法实现更节省内存，而链地址法实现更易于处理动态数据。代码结构清晰，错误处理完善，是一个高质量的哈希表实现。


unique_manager.h

这是一个唯一键值管理器的头文件实现，采用Trie树(前缀树)数据结构来高效管理字符串键。以下是关键设计分析：

    ‌数据结构设计‌：

    unique_key_node结构体构成Trie树节点，支持37个子节点(a-z0-9+_字符集)
    unique_index作为树根容器，维护节点计数和根指针
    每个节点存储字符(key_char)、层级(level)和行ID(row_id)

    ‌核心功能接口‌：

    init_unique_manager：初始化空索引树
    insert_unique_key：插入键值对(时间复杂度O(L)，L为键长)
    search_unique_key：精确查找键对应的行ID
    delete_unique_key：惰性删除节点(标记而非立即释放)
    free_unique_manager：递归释放整棵树内存

    ‌工程特性‌：

    使用标准C头文件保护宏防止重复包含
    严格的函数原型声明，参数和返回值类型明确
    注释清晰标注各结构体和函数用途
    设计为无状态模块，通过指针参数操作数据结构

技术总结：该实现特别适合代理服务器需要快速路由查找的场景，Trie树结构对字符串前缀匹配有天然优势。size_t类型的使用确保能处理大容量键值存储。

unique_manager.c

这是一个基于Trie树(字典树)实现的唯一键值管理器，主要用于高效存储和检索字符串键与数值的映射关系。以下是代码的核心实现分析：

    ‌数据结构设计‌：

    使用37叉树结构(a-z0-9+_)存储键字符
    每个节点包含字符标记(key_char)和关联的行ID(row_id)
    通过level字段记录节点深度

    ‌核心算法实现‌：

    字符转换：_char_to_index将字符映射到0-36的数组下标
    递归插入：_insert_recursive实现深度优先的键插入
    惰性删除：_delete_recursive仅清除row_id标记
    内存回收：删除时自动释放无子节点的分支

    ‌标准库使用‌：

    使用calloc进行节点内存分配和清零初始化
    通过free实现递归内存释放
    依赖stdio.h进行测试输出

技术总结：该实现特别适合需要快速前缀匹配的场景，如代理服务器的路由表管理。相比哈希表方案，Trie树在以下方面具有优势：

    天然支持前缀搜索
    无哈希冲突问题
    键有序存储

index_manager.h

该头文件实现了一个基于多叉树的唯一索引管理系统，主要包含以下核心设计：

一、数据结构设计

    索引节点结构

    unique_index_key_node：37叉树节点（a-z0-9+_字符集），包含：
        字符键值key_char和层级level
        子节点指针数组children[CHAR_INDEX_CHILDREN_COUNT]
        指向10叉树的行ID树指针row_id_tree

    行ID节点结构

    index_row_id_key_node：10叉树节点（0-9数字），包含：
        数字键值key_char和层级level
        子节点指针数组children[UNSIGNED_INDEX_CHILDREN_COUNT]
        实际行ID存储row_id

二、核心功能模块

    索引管理

    创建/初始化/释放管理器（create_unique_index_manager等）
    支持键值插入/查询/删除（insert_unique_index_key等）
    行ID级联操作（insert_row_id/delete_row_id）

    遍历查询

    递归遍历所有键（traverse_keys_helper）
    分页获取行ID（get_row_ids_paginated）
    支持回调函数处理查询结果

三、关键技术点

    内存管理

    显式释放函数（free_index_tree）防止内存泄漏
    多级指针操作确保树结构完整性

    类型安全

    字符到索引的转换函数（unique_index_char_to_index）
    严格的节点空值检查（unique_index_is_leaf）

该设计通过两级树结构（37叉树+10叉树）实现高效索引，适用于需要快速查找和分页的场景。头文件使用标准的包含保护宏（#ifndef INDEX_MANAGER_H）防止重复包含。

lock_manager.h

该头文件实现了一个基于哈希表的行级锁管理系统，主要包含以下核心设计：

一、数据结构设计

    行锁结构(row_lock_t)

    包含互斥锁(pthread_mutex_t)和条件变量(pthread_cond_t)实现同步
    引用计数(refcount)支持锁重入机制
    行标识(row_id)和状态标记(is_used)实现精确锁定位

    哈希桶结构(lock_bucket_t)

    读写锁(pthread_rwlock_t)保护桶内并发访问
    固定大小数组(entries)存储行锁，采用开放寻址法解决冲突
    MAX_PROBE_DEPTH限制探测深度防止性能退化

二、关键功能设计

    锁管理接口

    创建/销毁管理器(create_lock_manager/free_lock_manager)负责资源生命周期
    获取锁(acquire_lock)实现阻塞等待和锁升级
    释放锁(release_lock)处理引用计数递减和唤醒等待者

三、并发控制策略

    两级锁机制

    外层读写锁控制桶访问(读多写少场景优化)
    内层互斥锁保护具体行锁状态
    条件变量实现高效线程唤醒

    安全边界

    REFCOUNT_MAX防止引用计数溢出
    固定大小哈希表避免动态扩容并发问题
    状态标记(is_used)确保锁对象安全复用

该设计适用于数据库等需要精细并发控制的场景，通过哈希分桶降低锁竞争，同时支持行级锁的等待/唤醒机制。实现时需注意避免重复加锁和死锁问题。

db_struct_data.h

该头文件定义了一个层次化的数据库管理系统架构，主要包含以下核心设计特性：

一、系统架构设计

    四级层级结构

    数据管理器→服务器→数据库→数据表构成完整管理链
    每级通过固定大小数组存储下级对象（如data_servers[DATA_SERVER_MAX_SIZE]）
    指针类型别名（如data_manager_point）增强代码可读性

二、核心数据结构

    表存储结构

    数据行采用双向链表设计（previous_node/next_node）支持高效遍历
    列结构支持三种数据类型（数值/字符串/二进制）通过value_type区分
    动态内存管理通过value_size和void* value实现

    索引系统

    二叉树索引（branch_node）维护行数据范围（*_start/*_end）
    集成唯一索引（unique_managers）和普通索引（unique_index_managers）管理器

三、关键技术特性

    跨平台支持

    内存对齐分配宏ALIGNED_ALLOC适配Windows/Linux差异
    线程安全设计包含pthread.h头文件

    安全机制

    集成加密模块（hybrid.h）
    字段名长度限制（DATA_COL_NAME_MAX_LEN）防止缓冲区溢出
    行级锁管理依赖lock_manager.h实现并发控制

该设计通过复合索引结构（二叉树+多级数组）实现高效数据检索，适用于需要支持分布式存储的事务型数据库系统。头文件使用标准的#ifndef保护宏防止重复包含。

在数据架构层面，该头文件定义了一个基于二叉树索引的数据库管理系统，主要包含以下核心模块设计：

一、二叉树索引系统

    节点管理

    根节点创建(create_branch_node_tree)通过size参数初始化存储容量
    子树创建(create_subtree)支持指定范围(start/end)和方向标记(is_left_subtree)
    动态扩容(expand_branch_node_tree)采用两倍增长策略优化性能

    数据操作

    存在性检查(exist_data)通过递归遍历验证target_id
    数据存取(set_data/get_data)结合深度参数(depth)实现层级定位
    并发控制通过lock_manager_t参数保证线程安全

二、数据存储结构

    行列模型

    数据行(data_row)包含row_id和字段数组(fields[])，关联父节点(parent_node)
    数据列(data_col)支持三种类型(数值/字符串/二进制)和唯一性标记
    内存管理包含创建/释放接口(create/free_data_col/row)

三、数据库管理系统

    层级架构

    数据管理器→服务器→数据库→数据表四级结构
    每级提供增删查改标准接口(如add/find/remove_data_server)
    表级绑定二叉树根节点(bind_data_table_root_branch_node)

    辅助功能

    递归打印(print_branch_node_tree)支持树结构可视化
    行信息输出(print_rows/row)提供数据调试能力
    资源释放接口(free_*)构成完整生命周期管理

该设计通过二叉树索引优化数据检索效率，结合行列存储模型实现结构化数据管理，适用于需要高效查询的事务处理场景。头文件采用模块化设计分离索引、存储和管理功能，但需注意，二叉树深度(MAX_DEPTH)可能影响递归性能（ 理想深度 <= 20 ）。

linked_list.h

这是一个线程安全的双向链表实现头文件，具有以下关键特性：

    数据结构设计

    双向链表节点(LINKED_LIST)包含前后指针(last_point/next_point)和父子指针(parent_point/child_head_point)
    支持通用数据存储(COMMON_MEMORY data)和动态大小管理(SIZE size)
    每个节点包含唯一ID(id)和名称标识(name)

    线程安全机制

    使用三个静态互斥锁(linked_id_mutex/pages_id_mutex/operate_mutex)保护关键操作
    原子ID生成函数(generate_threadsafe_*_id)防止并发冲突
    所有数据操作接口都设计为线程安全

    核心功能

    数据操作：插入(_insertLinkedList)、更新(updateLinkedList)、删除(deleteLinkedList)
    数据查询：支持正序/倒序查找(findByIdInASC/DESC)和分页查询(findById)
    内存管理：通过LINKED_LIST_PAGE_SIZE控制内存分配粒度

    扩展特性

    支持子链表(child_head_point)实现树形结构存储
    提供字符串处理辅助函数(getName/getStringData)
    分页机制(LINKED_LIST_PAGE_SIZE)优化大数据量处理

该设计适用于需要高并发访问的链式数据存储场景，如内存数据库索引、任务调度队列等。通过模块化设计将实现细节隐藏在.c文件中，对外提供简洁的操作接口。

（未完待续）

备注：当前项目，仍然处于优化测试阶段，相关功能会不断扩充与完善。




