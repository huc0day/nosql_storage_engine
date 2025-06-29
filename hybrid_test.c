#include "libs/hybrid.h"

int aes_rsa_hybrid_test_main(){
// 初始化OpenSSL
    OpenSSL_add_all_algorithms( );
    ERR_load_crypto_strings( );

    // 1. 生成RSA密钥对
    RSA        * rsa_key      = generate_rsa_keypair ( );
    const char * test_cases[] = {
            "Hello World!" ,
            "这是一个中文字符串测试" ,
            "Special chars: !@#$%^&*()" ,
            ""
    };

    // 2. 测试各种情况
    for ( int i = 0 ; i < sizeof ( test_cases ) / sizeof ( test_cases[ 0 ] ) ; i++ ) {
        const char * plaintext = test_cases[ i ];
        size_t pt_len = strlen ( plaintext );

        printf ( "\n=== 测试用例 %d ===\n" , i + 1 );
        printf ( "原始数据: %s\n" , plaintext );

        // 加密
        Buffer encrypted = hybrid_encrypt ( ( const unsigned char * ) plaintext , pt_len , rsa_key );
        if ( !encrypted.data ) {
            printf ( "加密失败!\n" );
            continue;
        }
        printf ( "加密成功，总长度: %zu bytes\n" , encrypted.length );
        print_hex ( "RSA密文段" , encrypted.data , RSA_size ( rsa_key ) );
        print_hex ( "IV向量段" , encrypted.data + RSA_size ( rsa_key ) , EVP_MAX_IV_LENGTH );

        // 解密
        Buffer decrypted = hybrid_decrypt ( encrypted.data , encrypted.length , rsa_key );
        if ( !decrypted.data ) {
            printf ( "解密失败!\n" );
        } else {
            printf ( "解密成功: %.*s\n" , ( int ) decrypted.length , decrypted.data );
            if ( memcmp ( plaintext , decrypted.data , pt_len ) == 0 ) {
                printf ( "✅ 加解密验证通过\n" );
            } else {
                printf ( "❌ 加解密验证失败\n" );
            }
            free ( decrypted.data );
        }
        free ( encrypted.data );
    }

    // 清理
    RSA_free ( rsa_key );
    EVP_cleanup( );
    return 0;
}
