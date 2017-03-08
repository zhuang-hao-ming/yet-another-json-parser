#ifndef JSON_H__
#define JSON_H__

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
	json_type type;
	double num;
} json_node;/*节点类*/


enum {
	JSON_PARSE_OK,
	JSON_EXPECT_VALUE,
	JSON_INVALID_VALUE,
	JSON_ROOT_NOT_SINGULAR,
	JSON_PARSE_NUMBER_TOO_BID
};/*json_parse 函数返回的错误码枚举类型*/

/*解析json的函数*/
int json_parse(json_node* node, const char* json);
/*获取json节点类型的函数*/
json_type json_get_type(json_node* node);
/*获取json数值的函数*/
double json_get_number(json_node* node);

#endif