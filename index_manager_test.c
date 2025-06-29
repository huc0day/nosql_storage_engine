#include <stdio.h>
#include "libs/index_manager.h" // 假设头文件包含上述函数声明

// 打印键值对的回调函数
void print_key_value(const char* key, size_t row_id) {
    printf("Key: %s -> RowID: %zu\n", key, row_id);
}

int index_manager_test_main() {

    // 1. 初始化索引管理器
    unique_index_manager* mgr = init_index_manager("user_email_index");

    // 2. 插入测试数据
    printf("插入测试数据...\n");
    insert_unique_key(mgr, "alice@example.com", 1001);
    insert_unique_key(mgr, "bob@example.com", 1002);
    insert_unique_key(mgr, "charlie+test@example.com", 1003);

    // 3. 查找测试
    printf("\n查找测试:\n");
    printf("alice@example.com -> %zu\n", find_by_key(mgr, "alice@example.com"));
    printf("not_exist@test.com -> %zu\n", find_by_key(mgr, "not_exist@test.com"));

    // 4. 遍历所有键值对
    printf("\n当前所有索引项:\n");
    traverse_keys(mgr, print_key_value);

    // 5. 删除测试
    printf("\n删除bob@example.com后:\n");
    delete_key(mgr, "bob@example.com");
    printf("bob@example.com -> %zu\n", find_by_key(mgr, "bob@example.com"));

    // 6. 再次遍历验证
    printf("\n更新后的索引项:\n");
    traverse_keys(mgr, print_key_value);

    // 7. 清理资源
    free_index_manager(mgr);

    // 1. 创建索引管理器
    index_row_id_key_node* root = NULL;

    // 2. 插入测试数据
    printf("插入测试数据...\n");
    insert_row_id(&root, 123);
    insert_row_id(&root, 456);
    insert_row_id(&root, 789);
    insert_row_id(&root, 1000000001);
    insert_row_id(&root, 222222222222222);

    // 3. 查找测试
    printf("\n查找测试:\n");
    printf("123存在? %s\n", find_row_id(root, 123) ? "是" : "否");
    printf("999存在? %s\n", find_row_id(root, 999) ? "是" : "否");

    // 4. 分页查询测试
    printf("\n分页查询测试(每页%d条):\n", PAGE_SIZE);
    size_t* results = NULL;
    int total = 0, count = 0;

    // 第一页
    init_paginated_query(&results, &total, &count);
    get_row_ids_paginated(root, &results, &total, 1, &count);
    printf("第1页(共%d条, 总数%d):\n", count, total);
    for (int i = 0; i < count; i++) {
        printf("%zu\n", results[i]);
    }
    free(results);

    // 5. 删除测试
    printf("\n删除456后:\n");
    delete_row_id(&root, 456);
    printf("456存在? %s\n", find_row_id(root, 456) ? "是" : "否");

    // 6. 清理资源
    free_index_tree(root);
    return 0;
}
