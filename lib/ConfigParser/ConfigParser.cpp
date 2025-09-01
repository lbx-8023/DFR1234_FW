/**
 * @file    ConfigParser.cpp
 * @brief   配置文件解析模块实现
 * 
 * @details 提供配置文件的键值对解析、查询功能，
 *          支持去除字符串空格、大小写不敏感查询、带默认值查询等特性，
 *          适用于解析INI格式或纯文本键值对配置。
 */

#include <cctype>  // C++ 风格
#include "ConfigParser.h"
#include "Arduino.h"
/**
 * @brief 去除字符串前后空格
 * 
 * @param str 待处理的字符串指针
 * @return char* 处理后的字符串指针
 * @details 1. 跳过前导空白字符（空格、制表符等）
 *          2. 处理全空字符串的情况
 *          3. 去除尾部空白字符并截断字符串
 */
 char* trim(char* str) {
    if (!str) return NULL;
    
    // 去除前导空白
    char* start = str;
    while (*start && isspace((unsigned char)*start)) start++;
    
    // 处理全空字符串的情况
    if (*start == '\0') {
        *str = '\0';
        return str;
    }

    // 去除尾部空白
    char* end = str + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;

    // 移动处理后的字符串到起始位置
    size_t len = end - start + 1;
    if (start != str) {
        memmove(str, start, len);
    }
    str[len] = '\0';

    return str;
}

/**
 * @brief 解析键值对字符串
 * 
 * @param input 输入的配置字符串（多行，每行一个键值对）
 * @param kv_pairs 存储键值对的结构体数组
 * @param max_entries 最大解析条目数
 * @return int 成功解析的键值对数量（-1表示内存分配失败）
 * @details 1. 支持换行符（\n/\r）分割行
 *          2. 动态查找冒号作为键值分隔符
 *          3. 自动去除键和值的前后空格
 *          4. 忽略空行和无效行（无冒号或值为空）
 */
int parse_kv(const char* input, KeyValue* kv_pairs, int max_entries) {
    char* input_copy = strdup(input);
    if (!input_copy) return -1;

    int count = 0;
    char* line = strtok(input_copy, "\n\r");  // 处理不同换行格式
    
    while (line && count < max_entries) {
        char* key_end = NULL;
        char* value_start = NULL;

        // 动态查找键值分界符
        for (char* p = line; *p; p++) {
            if (*p == ':' && !key_end) {
                key_end = p;
                // 继续查找后续的非空白字符作为值起点
                for (value_start = p+1; *value_start; value_start++) {
                    if (!isspace((unsigned char)*value_start)) break;
                }
                break;
            }
        }

        // 验证是否有效键值对（存在冒号且值非空）
                if (key_end && *value_start) {
            *key_end = '\0';
            char* key = trim(line);
            char* value = trim(value_start);

            // 检查键和值是否都非空
            if (*key && *value) {  
                strncpy(kv_pairs[count].key, key, MAX_KEY_LEN-1);
                strncpy(kv_pairs[count].value, value, MAX_VALUE_LEN-1);
                kv_pairs[count].key[MAX_KEY_LEN-1] = '\0';
                kv_pairs[count].value[MAX_VALUE_LEN-1] = '\0';
                count++;
            } else {
                // 如果键或值为空，跳过该行
                printf("跳过无效行（键或值为空）: %s\n", line);
            }
        } else {
            // 如果没有找到冒号或值为空，跳过该行
            printf("跳过无效行: %s\n", line);
        }

        line = strtok(NULL, "\n\r");
    }

    free(input_copy);
    return count;
}

/**
 * @brief 大小写不敏感查询键值对
 * 
 * @param kv_pairs 键值对结构体数组
 * @param count 有效键值对数量
 * @param key 待查询的键
 * @return char* 对应的值（NULL表示未找到）
 * @details 使用strcasecmp实现忽略大小写比较，返回第一个匹配项
 */
char *get_value_case_insensitive( KeyValue *kv_pairs, int count, const char *key)
{
    for (int i = 0; i < count; i++)
    {
        if (strcasecmp(trim(kv_pairs[i].key),key) == 0)
        {
            return kv_pairs[i].value;
        }
    }
    return NULL;
}

/**
 * @brief 精确匹配查询键值对
 * 
 * @param kv_pairs 键值对结构体数组
 * @param count 有效键值对数量
 * @param key 待查询的键
 * @return char* 对应的值（NULL表示未找到）
 * @details 使用strcmp进行精确匹配（区分大小写）
 */
 char *get_value_by_key( KeyValue *kv_pairs, int count, const char *key)
{
    for (int i = 0; i < count; i++)
    {
        if (strcmp(trim(kv_pairs[i].key),key) == 0)
        {
            return kv_pairs[i].value;
        }
    }
    return NULL;
}
/**
 * @brief 带默认值的键值对查询
 * 
 * @param kv_pairs 键值对结构体数组
 * @param count 有效键值对数量
 * @param key 待查询的键
 * @param default_val 默认值
 * @return const char* 对应的值或默认值
 * @details 优先返回查询到的值，未找到时返回默认值（避免空指针）
 */
const char *get_value_with_default( KeyValue *kv_pairs, int count, const char *key, const char *default_val)
{
    const char *val = get_value_by_key(kv_pairs, count, key);
    return val ? val : default_val;
}