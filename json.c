#include "json.h"
#include <stdlib.h> /*NULL strtod*/
#include <assert.h> /*assert*/
#include <errno.h> /*errno ERANGE*/
#include <math.h>/*HUGE_VAL*/

#define EXPECT(c, ch) do {assert(*((c)->json) == (ch)); (c)->json++;} while(0)

typedef struct {
	const char* json;
} json_context;

/*解析json字符串中的空白*/
static void json_parse_whitespace(json_context* c) {
	const char* p = c->json;
	while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '\f' || *p == '\v') {
		p++;
	}
	c->json = p;
}

/*解析字面量json*/
/*todo 分析我的写法和别人写法的优劣*/
static int json_parse_literal(json_context* c, json_node* node, const char* literal, json_type type) {
	
	size_t i = 0;
	for (;*literal;i++, literal++) {
		if (c->json[i] != *literal) {
			return JSON_INVALID_VALUE;
		}		
	}
	c->json += i;
	node->type = type;
	return JSON_PARSE_OK;
}



#define IS_DIGITAL(ch) ((ch) >= '0' && (ch) <= '9')
#define IS_DIGITAL_1_To_9(ch) ((ch) >= '1' && (ch) <= '9')
static int json_parse_number(json_context* c, json_node* node) {
	const char* p = c->json;
	/*要诀是如果分支上没有字符,那么分支一定合法不要考虑它*/
	if(*p == '-') {
		p++;
	}
	if (*p == '0') {
		p++;
	} else {
		if (IS_DIGITAL_1_To_9(*p)) {
			p++;
		} else {
			return JSON_INVALID_VALUE;
		}
		for(;IS_DIGITAL(*p); p++);
	}
	if (*p == '.') {
		p++;	
		if (IS_DIGITAL(*p)) {
			p++;
		} else {
			return JSON_INVALID_VALUE;
		}
		for(;IS_DIGITAL(*p); p++);
	}
	if (*p == 'e' || *p == 'E') {
		p++;
		if (*p == '+' || *p == '-') {
			p++;
		}
		if (IS_DIGITAL(*p)) {
			p++;
		} else {
			return JSON_INVALID_VALUE;
		}
		for(;IS_DIGITAL(*p); p++);
	}
	errno = 0;
	node->num = strtod(c->json, NULL);
	if (errno == ERANGE && (node->num > HUGE_VAL || node->num < -HUGE_VAL)) {
		return JSON_PARSE_NUMBER_TOO_BID;
	}
	node->type = JSON_NUMBER;
	c->json = p;
	return JSON_PARSE_OK;

}

/*解析json字符串中的值*/
static int json_parse_value(json_context* c, json_node* node) {
	switch (*c->json) {
		case 'n': return json_parse_literal(c, node, "null", JSON_NULL);/*"null"*/
		case 'f': return json_parse_literal(c, node, "false", JSON_FALSE);/*"null"*/
		case 't': return json_parse_literal(c, node, "true", JSON_TRUE);/*"null"*/
		case '\0': return JSON_EXPECT_VALUE; /*空json*/
		default: return json_parse_number(c, node);
	}
}

/*解析json的函数*/
int json_parse(json_node* node, const char* json) {
	json_context c;
	assert(node != NULL);
	c.json = json;
	node->type = JSON_NULL;
	json_parse_whitespace(&c);

	int ret = json_parse_value(&c, node);
	if (ret == JSON_PARSE_OK) {
		json_parse_whitespace(&c);
		if (*c.json != '\0') {
			node->type = JSON_NULL;
			return JSON_ROOT_NOT_SINGULAR;
		}
	} else {
		return ret;
	}
}
/*获取json节点类型的函数*/
json_type json_get_type(json_node* node) {
	assert(node != NULL);
	return node->type;
}
/*获取json节点数值的函数*/
double json_get_number(json_node* node) {
	assert(node != NULL && node->type == JSON_NUMBER);
	return node->num;
}