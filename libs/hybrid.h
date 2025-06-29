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


#ifndef HYBRID_H  // 头文件保护宏
#define HYBRID_H

//包含头文件
#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/md5.h>
#include <openssl/err.h>
#include <string.h>
//
//
//定义常量
#define AES_KEY_SIZE   32
#define IV_SIZE        16
#define RSA_KEY_SIZE   4096
#define CHUNK_SIZE  (16 * 1024)  // 16KB分块处理
//
//
//定义数据结构
typedef struct {
    unsigned char * data;
    size_t length;
} Buffer;

RSA * generate_rsa_keypair ( ) ;

void print_hex ( const char * label , const unsigned char * buf , size_t len ) ;

Buffer hybrid_encrypt ( const unsigned char * plaintext , size_t pt_len , RSA * rsa_pubkey ) ;

Buffer hybrid_decrypt ( const unsigned char * ciphertext , size_t ct_len , RSA * rsa_privkey ) ;

// 字符串MD5计算（对应php的md5()）
char * md5 ( const char * str ) ;

void print_bin_data ( unsigned char * encrypt_data ) ;

char * get_bin_data ( unsigned char * encrypt_data ) ;

void handle_openssl_error ( ) ;

RSA * load_rsa_pubkey ( const char * pubkey_path ) ;

void free_buffer ( Buffer * buf ) ;

int create_rsa_key_pem_file_ ( ) ;

#endif
