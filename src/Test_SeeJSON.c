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
}

static void test_for_parse_root_error()
{
    json_node node;
    node.type = JSON_FALSE;
    TEST_INT(JSON_PARSE_ROOT_ERROR,json_parse(&node,"null x"));
    TEST_INT(JSON_NULL,json_get_type(&node));
}

static void test_parse()
{
    test_for_parse_null();
    test_for_parse_true();
    test_for_parse_false();
    test_for_parse_expect_value();
    test_for_parse_invalid_value();
    test_for_parse_root_error();
}

int main()
{
    test_parse();
    printf("%d/%d (%3.2f%%) Cases Passed. \n",cases_passed,cases_total,cases_passed*100.0/cases_total);
    system("pause");
    return status;
}
