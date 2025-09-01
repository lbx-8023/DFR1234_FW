
/**
 * @file    ConfigParser.h
 * @brief   配置文件解析模块头文件
 * 
 * @details 提供配置文件的键值对解析及查询接口，
 *          支持大小写不敏感查询、带默认值查询等功能，
 *          适用于解析INI格式或纯文本键值对配置。
 */
#pragma once
// #ifndef CONFIG_PARSER_H
// #define CONFIG_PARSER_H
#include <cstring>
#include <cctype>
#include "esp_log.h"

// 配置解析器参数定义
 ///< 最大键值对数量（可根据需求调整）
#define MAX_ENTRIES 10     
 ///< 键的最大长度（含终止符）
#define MAX_KEY_LEN 30  
 ///< 值的最大长度（含终止符）    
#define MAX_VALUE_LEN 30    

/**
 * @brief 键值对结构体
 * 
 * @details 存储配置文件中的键和值，
 *          键和值均包含长度限制，防止缓冲区溢出。
 */
typedef struct {
    char key[MAX_KEY_LEN];    ///< 配置项的键
    char value[MAX_VALUE_LEN];///< 配置项的值
} KeyValue;

/**
 * @brief 解析键值对字符串
 * 
 * @param input 输入的配置字符串（多行格式，每行一个键值对）
 * @param kv_pairs 用于存储解析结果的键值对结构体数组
 * @param max_entries 允许解析的最大键值对数量
 * @return int 成功解析的键值对数量（-1表示内存分配失败）
 */
int parse_kv(const char *input, KeyValue *kv_pairs, int max_entries);

/**
 * @brief 精确匹配查询键值对（区分大小写）
 * 
 * @param kv_pairs 键值对结构体数组
 * @param count 有效键值对数量
 * @param key 待查询的键（需与配置中的键完全一致）
 * @return char* 对应的值（NULL表示未找到）
 */
char *get_value_by_key( KeyValue *kv_pairs, int count, const char *key);


/**
 * @brief 大小写不敏感查询键值对
 * 
 * @param kv_pairs 键值对结构体数组
 * @param count 有效键值对数量
 * @param key 待查询的键（不区分大小写）
 * @return char* 对应的值（NULL表示未找到）
 * @example 查询"wifi_ssid"可匹配"WiFi_SSID"或"wifi_ssid"
 */
char *get_value_case_insensitive( KeyValue *kv_pairs, int count, const char *key);

/**
 * @brief 带默认值的键值对查询
 * 
 * @param kv_pairs 键值对结构体数组
 * @param count 有效键值对数量
 * @param key 待查询的键
 * @param default_val 未找到键时返回的默认值
 * @return const char* 对应的值或默认值（避免返回NULL）
 */
const char *get_value_with_default( KeyValue *kv_pairs, int count, const char *key, const char *default_val);

// #endif
