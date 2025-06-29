#include "libs/hybrid.h"

int hybrid_test_main(){
    //
    char * md5_value = md5("12345678");
    printf("\n%s\n",md5_value);
    free(md5_value);
    //
    return 0;
}
