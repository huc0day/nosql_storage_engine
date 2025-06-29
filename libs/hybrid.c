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
