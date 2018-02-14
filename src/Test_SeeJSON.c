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
            fprintf(stderr,"%s:%d\nExpect for: " format " \nBut actually get: " format "\n",__FILE__,__LINE__,expect,fact); \
            status = 1; \
        }   \
    }while(0)


#define TEST_INT(expect,fact)\
            TEST_CORE((expect)==(fact),expect,fact,"%d")


#define TEST_DOUBLE(expect,fact)\
            TEST_CORE((expect)==(fact),expect,fact,"%.17g")


#define TEST_NUMBER(expect,json)  \
    do{ \
        json_node node; \
        TEST_INT(JSON_PARSE_SUCCESS,json_parse(&node,json));\
        TEST_INT(JSON_NUMBER,json_get_type(&node));\
        TEST_DOUBLE(expect,json_get_number(&node));\
    }while(0)


/* Encapsulate the Test method */
#define TEST_ERROR(error,json)\
    do{\
        json_node node;\
        node.type = JSON_FALSE;\
        TEST_INT(error,json_parse(&node,json));\
        TEST_INT(JSON_NULL,json_get_type(&node));\
    }while(0)


/**********************************************
                    TestCases
 *********************************************/

static void test_for_parse_null()
{
    json_node node;
    node.type = JSON_FALSE;
    TEST_INT(JSON_PARSE_SUCCESS,json_parse(&node,"null"));
    TEST_INT(JSON_NULL,json_get_type(&node));
}

static void test_for_parse_true()
{
    json_node node;
    node.type = JSON_FALSE;
    TEST_INT(JSON_PARSE_SUCCESS,json_parse(&node,"true"));
    TEST_INT(JSON_TRUE,json_get_type(&node));
}

static void test_for_parse_false()
{
    json_node node;
    node.type = JSON_FALSE;
    TEST_INT(JSON_PARSE_SUCCESS,json_parse(&node,"false"));
    TEST_INT(JSON_FALSE,json_get_type(&node));
}

static void test_for_parse_expect_value()
{
    json_node node;

    node.type = JSON_FALSE;
    TEST_INT(JSON_PARSE_EXPECT_VALUE,json_parse(&node,""));
    TEST_INT(JSON_NULL,json_get_type(&node));

    node.type = JSON_FALSE;
    TEST_INT(JSON_PARSE_EXPECT_VALUE,json_parse(&node," "));
    TEST_INT(JSON_NULL,json_get_type(&node));
}

static void test_for_parse_invalid_value()
{
    json_node node;

    node.type = JSON_FALSE;
    TEST_INT(JSON_PARSE_INVALID_VALUE,json_parse(&node,"nux"));
    TEST_INT(JSON_NULL,json_get_type(&node));

    node.type = JSON_FALSE;
    TEST_INT(JSON_PARSE_INVALID_VALUE,json_parse(&node,"?"));
    TEST_INT(JSON_NULL,json_get_type(&node));

    /// Test for INVALID NUMBER Type
    TEST_ERROR(JSON_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(JSON_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(JSON_PARSE_INVALID_VALUE, ".123");
    TEST_ERROR(JSON_PARSE_INVALID_VALUE, "1.");
    TEST_ERROR(JSON_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(JSON_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(JSON_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(JSON_PARSE_INVALID_VALUE, "nan");
}

static void test_for_parse_root_error()
{
    json_node node;
    node.type = JSON_FALSE;
    TEST_INT(JSON_PARSE_ROOT_ERROR,json_parse(&node,"null x"));
    TEST_INT(JSON_NULL,json_get_type(&node));

    TEST_ERROR(JSON_PARSE_ROOT_ERROR, "0123"); /* after zero should be '.' or nothing */
    TEST_ERROR(JSON_PARSE_ROOT_ERROR, "0x0");
    TEST_ERROR(JSON_PARSE_ROOT_ERROR, "0x123");
}

static void test_for_parse_number()
{
    /// Test for different types of numbers
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

static void test_for_number_overflow()
{
    TEST_ERROR(JSON_PARSE_NUMBER_OVERFLOW,"1e309");
    TEST_ERROR(JSON_PARSE_NUMBER_OVERFLOW,"-1e309");
}

static void test_parse()
{
    test_for_parse_null();
    test_for_parse_true();
    test_for_parse_false();
    test_for_parse_expect_value();
    test_for_parse_invalid_value();
    test_for_parse_root_error();
    test_for_parse_number();
    test_for_number_overflow();
}

int main()
{
    test_parse();
    printf("%d/%d (%3.2f%%) Cases Passed. \n",cases_passed,cases_total,cases_passed*100.0/cases_total);
    system("pause");
    return status;
}
