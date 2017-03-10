#ifndef JSON_H__
#define JSON_H__
#include <stdlib.h>
typedef enum {
	JSON_NULL,
	JSON_FALSE,
	JSON_TRUE,
	JSON_NUMBER,
	JSON_STRING,
	JSON_ARRAY,
	JSON_OBJECT
} json_type;/*节点类型的枚举类*/

typedef struct {
	union {
		struct {
			char* s;
			size_t len;
		} s;
		double num;		
	} u;
	json_type type;	
} json_node;/*节点类*/


enum {
	JSON_PARSE_OK,/*0*/
	JSON_EXPECT_VALUE,/*1*/
	JSON_INVALID_VALUE,/*2*/
	JSON_ROOT_NOT_SINGULAR,/*3*/
	JSON_PARSE_NUMBER_TOO_BID,/*4*/
	JSON_PARSE_MISS_QUOTATION_MARK,/*5*/
	JSON_PARSE_INVALID_STRING_ESCAPE,/*6*/
	LEPT_PARSE_INVALID_STRING_CHAR,/*7*/
	JSON_PARSE_INVALID_UNICODE_HEX,
	JSON_PARSE_UNICODE_SURROGATE,
};/*json_parse 函数返回的错误码枚举类型*/

/*解析json的函数*/
int json_parse(json_node* node, const char* json);
/*获取json节点类型的函数*/
json_type json_get_type(json_node* node);
/*获取json数值的函数*/
double json_get_number(const json_node* node);
/*设置节点数值的函数*/
void json_set_number(json_node* node, double num);
/*获取节点boolean值的函数*/
int json_get_boolean(const json_node* node);
/*设置节点boolean值的函数*/
void json_set_boolean(json_node* node, int b);
/*获取节点字符串的函数*/
const char* json_get_string(const json_node* node);
/*获取节点字符串长度的函数*/
size_t json_get_string_length(const json_node* node);
/*设置节点字符串的函数*/
void json_set_string(json_node* node, const char* str, size_t len);

/*初始化 宏*/
#define json_init(node) do { (node)->type = JSON_NULL; } while(0)
/*释放节点内存*/
void json_free(json_node* node);

#endif