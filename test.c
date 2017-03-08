#include <stdio.h>
#include "json.h"

static int test_count = 0;
static int test_pass = 0;
static int main_ret = 0;
/*
疑问:
1. char* s = "hello" "haoming"; 这样的写法是合法的,所以在宏里面可以

写这样的语句"%s:%d: expect: " format "actual: " format "\n"

2. 宏内换行要用\来分割,而且\和\n之间不可以跟任何字符,注释也不可以

3. 宏函数的参数尽量用括号括起来,保证参数优先级正确

4. 包含多条语句的宏,应该包括在do {...} while(0)中,以防止文本替换产生的bug

5. ## 和 # 命令

*/
#define EXPECT_EQ(equality, expect, actual, format) \
	do {\
		test_count++;\
		if (equality) { \
			test_pass++; \
		} else { \
			fprintf(stderr, "%s:%d: expect: " #expect " "  format " actual: " #actual " " format "\n", __FILE__, __LINE__, expect, actual); \
			main_ret = 1; \
		}\
		} while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ((expect) == (actual), expect, actual, "%f")

#define TEST_NUMBER(expect, json)\
	do {\
		json_node node;\
		node.type = JSON_NULL;\
		EXPECT_EQ_INT(JSON_PARSE_OK,json_parse(&node, json));\
		EXPECT_EQ_INT(JSON_NUMBER,json_get_type(&node));\
		EXPECT_EQ_DOUBLE(expect, json_get_number(&node));\
	} while (0)

#define TEST_ERROR(error, json)\
	do {\
		json_node node;\
		node.type = JSON_FALSE;\
		EXPECT_EQ_INT(error,json_parse(&node, json));\
		EXPECT_EQ_INT(JSON_NULL,json_get_type(&node));\
	} while (0)

static void test_parse_number() {
	TEST_NUMBER(0.0, "0");
	TEST_NUMBER(0.0, "-0");
	TEST_NUMBER(0.0, "0.0");
	TEST_NUMBER(0.0, "-0.0");
	TEST_NUMBER(1.0, "1");
	TEST_NUMBER(1.0, "1.0");
	TEST_NUMBER(-1.0, "-1.0");
	TEST_NUMBER(1E10, "1E10");
	TEST_NUMBER(1E+10, "1E+10");
	TEST_NUMBER(1e10, "1e10");
	TEST_NUMBER(1e+10, "1e+10");



}

static void test_parse_expect_value() {
	
	TEST_ERROR(JSON_EXPECT_VALUE, "  ");
	TEST_ERROR(JSON_EXPECT_VALUE, "");
}

static void test_parse_invalid() {
	
	TEST_ERROR(JSON_INVALID_VALUE, " a ");
	TEST_ERROR(JSON_INVALID_VALUE, "a");
	/*invalid number*/
	
	TEST_ERROR(JSON_INVALID_VALUE, "+012");


	
}

static void test_parse_number_too_big() {
	TEST_ERROR(JSON_PARSE_NUMBER_TOO_BID, "1e309");
}

static void test_parse_not_singular() {	
	TEST_ERROR(JSON_ROOT_NOT_SINGULAR, "null a");
	/**/
	TEST_ERROR(JSON_ROOT_NOT_SINGULAR, "012");
}

static void test_parse_null() {

	json_node node;
	node.type = JSON_TRUE;
	EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&node, "  	null	 "));
	EXPECT_EQ_INT(JSON_NULL,json_get_type(&node)); 

}

static void test_parse_false() {
	json_node node;
	node.type = JSON_NULL;
	EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&node, "false"));
	EXPECT_EQ_INT(JSON_FALSE, json_get_type(&node));
}

static void test_parse_true() {
	json_node node;
	node.type = JSON_NULL;
	EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&node, "true"));
	EXPECT_EQ_INT(JSON_TRUE, json_get_type(&node));
}


static void test_parse() {
	test_parse_expect_value();
	test_parse_invalid();
	test_parse_not_singular();

	test_parse_null();
	test_parse_false();
	test_parse_true();

	test_parse_number();
	test_parse_number_too_big();
}


int main() {
	
	test_parse();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);				
	return main_ret;
}