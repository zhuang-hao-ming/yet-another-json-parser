#include "json.h"
#include <stdlib.h> /*NULL strtod*/
#include <assert.h> /*assert*/
#include <errno.h> /*errno ERANGE*/
#include <math.h>/*HUGE_VAL*/
#include <string.h> /*memcpy*/
#define EXPECT(c, ch) do {assert(*((c)->json) == (ch)); (c)->json++;} while(0)

typedef struct {
	const char* json;
	char* stack;
	size_t size;/*栈容量*/
	size_t top;/*栈顶偏移的字节数目*/
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
	node->u.num = strtod(c->json, NULL);
	if (errno == ERANGE && (node->u.num > HUGE_VAL || node->u.num < -HUGE_VAL)) {
		return JSON_PARSE_NUMBER_TOO_BID;
	}
	node->type = JSON_NUMBER;
	c->json = p;
	return JSON_PARSE_OK;

}



#ifndef JSON_PARSE_STACK_INIT_SIZE
#define JSON_PARSE_STACK_INIT_SIZE 256
#endif

static void* json_context_push(json_context* c, size_t size) {
	void* ret;
	assert(size > 0);
	if (c->top + size >= c->size) {/*如果放入size字节,栈的容量不足或者已满,则增加栈大小*/
		if (c->size == 0) {
			c->size = JSON_PARSE_STACK_INIT_SIZE;
		}
		while (c->top + size >= c->size) {
			c->size += c->size >> 1;/*每次扩大为原来的1.5倍*/
		}
		c->stack = (char*)realloc(c->stack, c->size);
	}
	ret = c->stack + c->top; /*返回push前的栈顶,其实就是返回插入数据的起始位置*/
	c->top += size;
	return ret;
}

static void* json_context_pop(json_context* c, size_t size) {
	assert(c->top >= size);/*c->top的值代表了当前栈已有的数据量,弹栈前,至少要保证,已有的量比要弹出的量多*/
	return c->stack + (c->top -= size);/*弹出数据,只要把top减少即可, 然后返回栈顶地址*/
}

#define PUTC(c, ch) do { *(char*)(json_context_push(c, sizeof(char))) = (ch); } while(0)

/*从p中读取四个16进制字符,解析出一个无符号整数码点*/
static const char* json_parse_hex4(const char* p, unsigned* u) {
	*u = 0;
	for (int i = 0; i < 4; i++) {
		*u <<= 4;
		char ch = *p++;
		if (ch >= '0' && ch <= '9') {
			*u |= ch - '0';
		} else if (ch >= 'a' && ch <= 'f') {
			*u |= ch - 'a' + 10;
		} else if (ch >= 'A' && ch <= 'F') {
			*u |= ch - 'A' + 10;
		} else {
			return NULL;
		}		
	}
	return p;	
}
/**
把unicode码点转换为utf-8
*/
static void json_encode_utf8(json_context* c, unsigned u) {
	if (u <= 0x7f) {/*127个字符 00000000-01111111*/
		/*一个字节*/
		PUTC(c, u & 0xff);/*取出低8位作为一个字节*/
	}
	else if (u <= 0x7ff) {/*两个字节 110x xxxx 10xx xxxx*/
		PUTC(c, 0xc0 | ((u >> 6) & 0xff));/*取从最低位开始的第6位到10位, 所以先右移动6位, 再取低5位, 但由于,高位都是0所以直接用0xff来取*/
		PUTC(c, 0x80 | (u & 0x3f));/*取低6位*/
	}
	else if (u <= 0xffff) {/* 1110 xxxx 10xx xxxx 10xx xxxx */
		PUTC(c, 0xe0 | ((u >> 12) & 0xff));
		PUTC(c, 0x80 | ((u >> 6) & 0x3f));
		PUTC(c, 0x80 | (u & 0x3f));
	}
	else {
		assert(u <= 0x10ffff);
		/* 1111 0xxx 10xx xxxx 10xx xxxx 10xx xxxx */
		PUTC(c, 0xf0 | ((u >> 18) & 0xff));
		PUTC(c, 0x80 | ((u >> 12) & 0x03f));
		PUTC(c, 0x80 | ((u >> 6) & 0x3f));
		PUTC(c, 0x80 | (u & 0x3f));
	}
}


static int json_parse_string(json_context* c, json_node* node) {
	size_t head = c->top;
	size_t len;
	unsigned u;
	unsigned u2;
	EXPECT(c, '\"');
	const char* p = c->json;
	for(;;) {
		char ch = *p++;
		switch (ch) {
			case '\"':
				len = c->top - head;/*插入的字符数目*/
				json_set_string(node, (const char*)json_context_pop(c, len), len);
				c->json = p;
				return JSON_PARSE_OK;
			case '\\':
				switch(*p++) {
					case 'n': PUTC(c, '\n'); break;
					case 't': PUTC(c, '\t'); break;
					case 'b': PUTC(c, '\b'); break;
					case 'f': PUTC(c, '\f'); break;
					case 'v': PUTC(c, '\v'); break;
					case 'r': PUTC(c, '\r'); break;
					case '\\': PUTC(c, '\\'); break;
					case '/': PUTC(c, '/'); break;
					case '\"': PUTC(c, '\"'); break;
					case 'u': 
						p = json_parse_hex4(p, &u);
						if (p == NULL) {
							c->top = head;
							return JSON_PARSE_INVALID_UNICODE_HEX;
						}
						if (u >= 0xD800 && u<= 0xDBFF) {/*BMP的保留字符*/
							if(*p++ != '\\') {
								c->top = head;
								return JSON_PARSE_UNICODE_SURROGATE;
							}
							if (*p++ != 'u') {
								c->top = head;
								return JSON_PARSE_UNICODE_SURROGATE;
							}
							p = json_parse_hex4(p, &u2);
							if (p == NULL) {
								c->top = head;
								return JSON_PARSE_INVALID_UNICODE_HEX;
							}
							if (u2 < 0xDC00 || u2 > 0xDFFFF) {
								c->top = head;
								return JSON_PARSE_INVALID_UNICODE_HEX;
							}
							u = (((u - 0xD800) << 10 ) | (u2 - 0xDC00)) + 0x10000;
						}
						json_encode_utf8(c, u);
						break;
					default:
						c->top = head;
						return JSON_PARSE_INVALID_STRING_ESCAPE;					
				}
				break;
			case '\0':
				c->top = head; /*解析出错,将栈顶还原为原来的栈顶*/
				return JSON_PARSE_MISS_QUOTATION_MARK;
			default:
				if ((unsigned char)ch < 0x20) {
					c->top = head;
					return LEPT_PARSE_INVALID_STRING_CHAR;
				}
				PUTC(c, ch);break;
		}
	}
}


/*解析json字符串中的值*/
static int json_parse_value(json_context* c, json_node* node) {
	switch (*c->json) {
		case 'n': return json_parse_literal(c, node, "null", JSON_NULL);/*"null"*/
		case 'f': return json_parse_literal(c, node, "false", JSON_FALSE);/*"null"*/
		case 't': return json_parse_literal(c, node, "true", JSON_TRUE);/*"null"*/
		case '\0': return JSON_EXPECT_VALUE; /*空json*/
		case '"': return json_parse_string(c, node);
		default: return json_parse_number(c, node);
	}
}

/*解析json的函数*/
int json_parse(json_node* node, const char* json) {
	assert(node != NULL);

	json_context c;
	c.json = json;
	c.stack = NULL;
	c.size = c.top = 0;	
	
	json_init(node);

	json_parse_whitespace(&c);
	int ret = json_parse_value(&c, node);

	assert(c.top == 0);
	free(c.stack);

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
double json_get_number(const json_node* node) {
	assert(node != NULL && node->type == JSON_NUMBER);
	return node->u.num;
}
/*释放内存的函数*/
void json_free(json_node* node) {
	assert(node != NULL);
	if (node->type == JSON_STRING) {
		free(node->u.s.s);
	}
	node->type = JSON_NULL;

}
/*设置节点为null的函数*/
#define json_set_null(node) json_free(node)

/*设置节点字符串的函数*/
void json_set_string(json_node* node, const char* str, size_t len) {
	assert(node != NULL && (str != NULL || len == 0));
	json_free(node);
	node->u.s.s = (char*)malloc(len+1);
	memcpy(node->u.s.s, str, len); // 动态分配内存
	node->u.s.s[len] = '\0';
	node->u.s.len = len;
	node->type = JSON_STRING;
}

/*获取节点字符串的函数*/
const char* json_get_string(const json_node* node) {
	assert(node != NULL && node->type == JSON_STRING);
	return node->u.s.s;
}
/*获取节点字符串长度的函数*/
size_t json_get_string_length(const json_node* node) {
	assert(node != NULL && node->type == JSON_STRING);
	return node->u.s.len;
}


/*设置节点数值的函数*/
void json_set_number(json_node* node, double num) {
	assert(node != NULL);
	json_free(node);
	node->u.num = num;
	node->type = JSON_NUMBER;
}
/*获取节点boolean值的函数*/
int json_get_boolean(const json_node* node) {
	assert(node != NULL);
	return node->type == JSON_TRUE;
}
/*设置节点boolean值的函数*/
void json_set_boolean(json_node* node, int b) {
	assert(node != NULL);
	json_free(node);
	node->type = b ? JSON_TRUE : JSON_FALSE;
}