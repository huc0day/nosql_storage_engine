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


#include "hybrid.h"
//
//

// 字符串MD5计算（对应php的md5()）
char * md5 ( const char * str ) {
    //
    unsigned char digest[MD5_DIGEST_LENGTH];
    //
    char * result = malloc ( 33 );
    if(result==NULL){
        return NULL;
    }
    //
    MD5 ( ( unsigned char * ) str , strlen ( str ) , digest );
    //
    for ( int i = 0 ; i < 16 ; i++ ){
        sprintf ( result + i * 2 , "%02x" , digest[ i ] );
    }
    //
    return result;
}
