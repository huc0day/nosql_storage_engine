#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "libs/data_type.h"
#include "libs/linked_list.h"

int db_test() {

    char tmpName[32];
    char tmpData[32];
    //
    LINKED_LIST_POINT page_head_point = NULL, head_point = NULL, last_point = NULL, foot_point = NULL;
    int loop_max = 1000000;
    for (int i = 0; i < loop_max; i++) {
        //
        snprintf(tmpName, 32, "%012d", (i + 1));
        snprintf(tmpData, 32, "%s", "this is data");
        //
        char *name = getName(tmpName);       //取缔用数组方式传递KEY
        char *data = getStringData(tmpData); //取缔用数组方式传递DATA
        unsigned long long data_size = (unsigned long long) (strlen(data) + 1); //'\0'需要占一个字节

        //新增一条记录（扩展方式）
        LINKED_LIST_POINT this_point = insertData(name, data, data_size, &page_head_point);
        //
        if (i == 0) {                //第一条记录
            head_point = this_point; //设置头指针
        }
        if (i == (loop_max - 1)) {   //最后一条记录
            foot_point = this_point; //设置尾指针
        }
        last_point = this_point; //将当前记录的指针作为下次循环的上一条记录的指针
    }

    //测试写入情况
//    LINKED_LIST_POINT test_point = head_point;
//    while (test_point != NULL) {
//        printf("\n%ld %s %s\n", test_point->id, test_point->name, test_point->data);  //打印ID为100000的记录内容
//        test_point = test_point->next_point;
//    }

    LINKED_LIST_POINT this_point = findByIdInASC(head_point, 100000);           //正向查找ID为100000（10万）的记录
    if (this_point == NULL) {
        printf("100000 is null\n");
        return 0;
    } else {
        printf("%ld %s %s\n", this_point->id, this_point->name, this_point->data);  //打印ID为100000的记录内容

    }
//
//    //测试修改数据，需要新开辟空间
    memset(tmpData, 0, 32);                                                 //重新初始化数组
    snprintf(tmpData, 32, "%s", "this is data !!!");                        //重新为数组赋值
    char *data = getStringData(tmpData);                                    //生成传递参数
    unsigned long long data_size = (unsigned long long) (strlen(data) + 1); //'\0'需要占一个字节的空间

    updateLinkedList(this_point, tmpData, data_size); //更新数据

    printf("%ld %s %s\n", this_point->id, this_point->name, this_point->data);
//
//    //测试修改数据，复用原有空间
    memset(tmpData, 0, 32);                               //重新初始化数组
    snprintf(tmpData, 32, "%s", "this is gril.");         //重新为数组赋值
    data = getStringData(tmpData);                        //生成传递参数
    data_size = (unsigned long long) (strlen(data) + 1);  //'\0'需要占一个字节的空间

    updateLinkedList(this_point, tmpData, data_size); //更新数据

    printf("%ld %s %s\n", this_point->id, this_point->name, this_point->data);
//
//    //测试分页索引方式查找数据
    LINKED_LIST_POINT point = findById(100000, page_head_point); //查找ID为100000的记录
    if (point == NULL) {
        printf("not found point\n");
    } else {
        printf("found point info : %ld %s %s\n", this_point->id, this_point->name, this_point->data);
    }
//
////    //删除数据
    deleteLinkedList(this_point);                             //当前指针指向ID为100000的记录
    this_point = findByIdInASC(head_point, 100000);           //正向查找ID为100000（10万）的记录
    if (this_point != NULL) {
        printf("Delete fail : %ld %s %s\n", this_point->id, this_point->name, this_point->data);  //打印ID为100000的记录内容
    } else {
        printf("Delete success\n"); //已成功删除
    }
//
//    //测试分页索引方式查找数据
    point = findById(100000, page_head_point); //查找ID为100000的记录
    if (point == NULL) {
        printf("not found point\n");
    } else {
        printf("found point info : %ld %s %s\n", this_point->id, this_point->name, this_point->data);
    }
//

    return 0;
}


