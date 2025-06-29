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


#include "libs/unique_manager.h"

//
int unique_manager_test_main ( ) {

    // 1. 初始化索引
    unique_index user_index;
    init_unique_index ( & user_index );

    // 2. 插入测试数据
    printf ( "插入操作:\n" );
    insert_unique_key ( & user_index , "user_1001" , 1001 );
    insert_unique_key ( & user_index , "user_1002" , 1002 );
    insert_unique_key ( & user_index , "admin" , 1 );

    // 3. 查询演示
    printf ( "\n查询操作:\n" );
    printf ( "admin -> %zu\n" , search_unique_key ( & user_index , "admin" ) );
    printf ( "user_1001 -> %zu\n" , search_unique_key ( & user_index , "user_1001" ) );
    printf ( "not_exist -> %zu\n" , search_unique_key ( & user_index , "not_exist" ) );

    // 4. 删除演示
    printf ( "\n删除操作:\n" );
    delete_unique_key ( & user_index , "user_1001" );
    printf ( "删除后查询 user_1001 -> %zu\n" , search_unique_key ( & user_index , "user_1001" ) );

    // 5. 内存清理
    destroy_unique_index ( & user_index );
    //
    return 0;
}


