/// Test_SeeJSON.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SeeJSON.h"
#include "SeeJSON.c"

static int cases_total  = 0;   /* Sum of Test Cases */
static int cases_passed = 0;   /* Sum of Cases passed the test */
static int status       = 0;   /* Return value of Main */

#define TEST_CORE(func,expect,fact,format)  \
    do{ \
        cases_total++;  \
        if(func)    \
            cases_passed++; \
        else{   \
            fprintf(stderr,"%s:%d\nExpect for: " format " \nBut actually get: " format "\n\n",__FILE__,__LINE__,expect,fact); \
            status = 1; \
        }   \
    }while(0)


#define TEST_INT(expect,fact)\
            TEST_CORE((expect)==(fact),expect,fact,"%d")


#define TEST_DOUBLE(expect,fact)\
            TEST_CORE((expect)==(fact),expect,fact,"%.17g")


#define TEST_STRING_IS_RIGHT(expect,fact,length)\
            TEST_CORE(sizeof(expect)-1==length && memcmp(expect,fact,length)==0,expect,fact,"%s")


#define TEST_TRUE(fact) \
            TEST_CORE((fact)!=0,"true","false","%s")


#define TEST_FALSE(fact) \
            TEST_CORE((fact)==0,"false","true","%s")


#define TEST_NUMBER(expect,json)  \
    do{ \
        json_node node; \
        json_init(&node); \
        TEST_INT(JSON_PARSE_SUCCESS,json_parse(&node,json));\
        TEST_INT(JSON_NUMBER,json_get_type(&node));\
        TEST_DOUBLE(expect,json_get_number(&node));\
        json_free(&node); \
    }while(0)


#define TEST_STRING(expect,json) \
            do{ \
                json_node node; \
                json_init(&node); \
                TEST_INT(JSON_PARSE_SUCCESS,json_parse(&node,json)); \
                TEST_INT(JSON_STRING,json_get_type(&node)); \
                TEST_STRING_IS_RIGHT(expect,json_get_string(&node),json_get_string_length(&node)); \
                json_free(&node); \
            }while(0)


#if defined(_MSC_VER)
#define TEST_SIZE_T(expect,fact) TEST_CORE((expect) == (fact), (size_t)expect, (size_t)fact, "%Iu")
#else
#define TEST_SIZE_T(expect,fact) TEST_CORE((expect) == (fact), (size_t)expect, (size_t)fact, "%zu")
#endif // defined


/* Encapsulate the Test method */
#define TESTER(error,json)\
    do{\
        json_node node;\
        json_init(&node); \
        node.type = JSON_FALSE;\
        TEST_INT(error,json_parse(&node,json));\
        TEST_INT(JSON_NULL,json_get_type(&node));\
        json_free(&node); \
    }while(0)


/**********************************************
                    TestCases
 *********************************************/

static void test_for_parse_null()
{
    json_node node;
    json_init(&node);
    json_set_bool(&node,0);
    TEST_INT(JSON_PARSE_SUCCESS,json_parse(&node,"null"));
    TEST_INT(JSON_NULL,json_get_type(&node));
    json_free(&node);
}

static void test_for_parse_true()
{
    json_node node;
    json_init(&node);
    json_set_bool(&node,0);
    TEST_INT(JSON_PARSE_SUCCESS,json_parse(&node,"true"));
    TEST_INT(JSON_TRUE,json_get_type(&node));
    json_free(&node);
}

static void test_for_parse_false()
{
    json_node node;
    json_init(&node);
    json_set_bool(&node,0);
    TEST_INT(JSON_PARSE_SUCCESS,json_parse(&node,"false"));
    TEST_INT(JSON_FALSE,json_get_type(&node));
    json_free(&node);
}

static void test_for_error()
{
    /* Test for PARSE_EXPECT_VALUE */
    TESTER(JSON_PARSE_EXPECT_VALUE,"");
    TESTER(JSON_PARSE_EXPECT_VALUE," ");

    /* Test for INVALID NUMBER Type */
    TESTER(JSON_PARSE_INVALID_VALUE,"nux");
    TESTER(JSON_PARSE_INVALID_VALUE,"?");
    TESTER(JSON_PARSE_INVALID_VALUE, "+0");
    TESTER(JSON_PARSE_INVALID_VALUE, "+1");
    TESTER(JSON_PARSE_INVALID_VALUE, ".123");
    TESTER(JSON_PARSE_INVALID_VALUE, "1.");
    TESTER(JSON_PARSE_INVALID_VALUE, "INF");
    TESTER(JSON_PARSE_INVALID_VALUE, "inf");
    TESTER(JSON_PARSE_INVALID_VALUE, "NAN");
    TESTER(JSON_PARSE_INVALID_VALUE, "nan");

    /* Test for PARSE_ROOT_ERROR */
    TESTER(JSON_PARSE_ROOT_ERROR,"null x");
    TESTER(JSON_PARSE_ROOT_ERROR, "0123"); /* after zero should be '.' or nothing */
    TESTER(JSON_PARSE_ROOT_ERROR, "0x0");
    TESTER(JSON_PARSE_ROOT_ERROR, "0x123");

    /* Test for PARSE_NUMBER_OVERFLOW */
    TESTER(JSON_PARSE_NUMBER_OVERFLOW,"1e309");
    TESTER(JSON_PARSE_NUMBER_OVERFLOW,"-1e309");

    /* Test for PARSE_NO_QUATATION_ERROR */
    TESTER(JSON_PARSE_NO_QUOTATION_ERROR,"\"");
    TESTER(JSON_PARSE_NO_QUOTATION_ERROR,"\"abc");

    /* Test for PARSE_INVALID_STRING_ESCAPE */
    TESTER(JSON_PARSE_INVALID_STRING_ESCAPE,"\"\\v\"");
    TESTER(JSON_PARSE_INVALID_STRING_ESCAPE,"\"\\'\"");
    TESTER(JSON_PARSE_INVALID_STRING_ESCAPE,"\"\\0\"");
    TESTER(JSON_PARSE_INVALID_STRING_ESCAPE,"\"\\x12\"");

    /* Test for PARSE_INVALID_STRING */
    TESTER(JSON_PARSE_INVALID_STRING,"\"\x01\"");
    TESTER(JSON_PARSE_INVALID_STRING,"\"\x1F\"");

    /* Test for PARSE_INVALID_UNICODE */
    TESTER(JSON_PARSE_INVALID_UNICODE, "\"\\u\"");
    TESTER(JSON_PARSE_INVALID_UNICODE, "\"\\u0\"");
    TESTER(JSON_PARSE_INVALID_UNICODE, "\"\\u01\"");
    TESTER(JSON_PARSE_INVALID_UNICODE, "\"\\u012\"");
    TESTER(JSON_PARSE_INVALID_UNICODE, "\"\\u/000\"");
    TESTER(JSON_PARSE_INVALID_UNICODE, "\"\\uG000\"");
    TESTER(JSON_PARSE_INVALID_UNICODE, "\"\\u0/00\"");
    TESTER(JSON_PARSE_INVALID_UNICODE, "\"\\u0G00\"");
    TESTER(JSON_PARSE_INVALID_UNICODE, "\"\\u0/00\"");
    TESTER(JSON_PARSE_INVALID_UNICODE, "\"\\u00G0\"");
    TESTER(JSON_PARSE_INVALID_UNICODE, "\"\\u000/\"");
    TESTER(JSON_PARSE_INVALID_UNICODE, "\"\\u000G\"");
    TESTER(JSON_PARSE_INVALID_UNICODE, "\"\\u 123\"");

    /* Test for PARSE_INVALID_UNICODE_SURROGATE */
    TESTER(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TESTER(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TESTER(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TESTER(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TESTER(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");

    /* Test for invalid value in array */
    TESTER(JSON_PARSE_INVALID_VALUE,"[1,]");
    TESTER(JSON_PARSE_INVALID_VALUE,"[\"a\", nul]");

    /* Test for PARSE_UNCOMPLETE_ARRAY_FORMAT */
    TESTER(JSON_PARSE_UNCOMPLETE_ARRAY_FORMAT,"[1");
    TESTER(JSON_PARSE_UNCOMPLETE_ARRAY_FORMAT,"[1}");
    TESTER(JSON_PARSE_UNCOMPLETE_ARRAY_FORMAT,"[1 3");
    TESTER(JSON_PARSE_UNCOMPLETE_ARRAY_FORMAT,"[[]");
}

static void test_for_parse_number()
{
    /// Test for different types of numbers
    //TEST_NUMBER(0.0,"000");
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */
    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_for_parse_string()
{
    TEST_STRING("","\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
    TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

static void test_for_parse_array()
{
    size_t i,j;
    json_node node;
    json_init(&node);
    TEST_INT(JSON_PARSE_SUCCESS,json_parse(&node,"[ ]"));
    TEST_INT(JSON_ARRAY,json_get_type(&node));
    TEST_SIZE_T(0,json_get_array_size(&node));
    json_free(&node);

    json_init(&node);
    TEST_INT(JSON_PARSE_SUCCESS, json_parse(&node, "[ null , false , true , 123 , \"abc\" ]"));
    TEST_INT(JSON_ARRAY, json_get_type(&node));
    TEST_SIZE_T(5, json_get_array_size(&node));
    TEST_INT(JSON_NULL,   json_get_type(json_get_array_element_by_index(&node, 0)));
    TEST_INT(JSON_FALSE,  json_get_type(json_get_array_element_by_index(&node, 1)));
    TEST_INT(JSON_TRUE,   json_get_type(json_get_array_element_by_index(&node, 2)));
    TEST_INT(JSON_NUMBER, json_get_type(json_get_array_element_by_index(&node, 3)));
    TEST_INT(JSON_STRING, json_get_type(json_get_array_element_by_index(&node, 4)));
    TEST_DOUBLE(123.0, json_get_number(json_get_array_element_by_index(&node, 3)));
    TEST_STRING_IS_RIGHT("abc", json_get_string(json_get_array_element_by_index(&node, 4)), json_get_string_length(json_get_array_element_by_index(&node, 4)));
    json_free(&node);

    json_init(&node);
    TEST_INT(JSON_PARSE_SUCCESS, json_parse(&node, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    TEST_INT(JSON_ARRAY, json_get_type(&node));
    TEST_SIZE_T(4, json_get_array_size(&node));
    for (i = 0; i < 4; i++) {
        json_node* a = json_get_array_element_by_index(&node, i);
        TEST_INT(JSON_ARRAY, json_get_type(a));
        TEST_SIZE_T(i, json_get_array_size(a));
        for (j = 0; j < i; j++) {
            json_node* e = json_get_array_element_by_index(a, j);
            TEST_INT(JSON_NUMBER, json_get_type(e));
            TEST_DOUBLE((double)j, json_get_number(e));
        }
    }
    json_free(&node);
}

static void test_for_access_null()
{
    json_node node;
    json_init(&node);
    json_set_string(&node,"a",1);
    json_set_null(&node);
    TEST_INT(JSON_NULL,json_get_type(&node));
    json_free(&node);
}

static void test_for_access_bool()
{
    json_node node;
    json_init(&node);
    json_set_string(&node,"a",1);
    json_set_bool(&node,1);
    TEST_TRUE(json_get_bool(&node));
    json_set_bool(&node,0);
    TEST_FALSE(json_get_bool(&node));
    json_free(&node);
}

static void test_for_access_number()
{
    json_node node;
    json_init(&node);
    json_set_string(&node,"a",1);
    json_set_number(&node,1234.5);
    TEST_DOUBLE(1234.5,json_get_number(&node));
    json_free(&node);
}

static void test_for_access_string()
{
    json_node node;
    json_init(&node);
    json_set_string(&node,"",0);
    TEST_STRING_IS_RIGHT("",json_get_string(&node),json_get_string_length(&node));
    json_set_string(&node,"hello",5);
    TEST_STRING_IS_RIGHT("hello",json_get_string(&node),json_get_string_length(&node));
    json_free(&node);
}

static void test_parse()
{
    test_for_error();
    test_for_parse_null();
    test_for_parse_true();
    test_for_parse_false();
    test_for_parse_number();
    test_for_access_string();
    test_for_access_null();
    test_for_access_bool();
    test_for_access_number();
    test_for_access_string();
    test_for_parse_array();
}

int main()
{
    test_parse();
    printf("%d/%d (%3.2f%%) Cases Passed. \n",cases_passed,cases_total,cases_passed*100.0/cases_total);
    system("pause");
    return status;
}
