/// SeeJSON.c

#include "SeeJSON.h"
#include <assert.h>
#include <stdlib.h>

#define CHECK(context,ch)   \
            do{ \
                assert(*context->json==(ch));   \
                context->json++;    \
            }while(0)


/// JSON String
typedef struct{
    const char* json;
}json_context;


static void json_parse_space(json_context* con)
{
    const char *p = con->json;
    while(*p==' ' || *p=='\t' || *p=='\n' || *p=='\r')
    {
        p++;
    }
    con->json = p;
}

static int json_parse_true(json_context* con,json_node* node)
{
    CHECK(con,'t');
    if(con->json[0]!='r' || con->json[1]!='u' || con->json[2]!='e')
    {
        return JSON_PARSE_INVALID_VALUE;
    }
    con->json += 3;
    node->type = JSON_TRUE;
    return JSON_PARSE_SUCCESS;
}

static int json_parse_false(json_context* con, json_node* node) {
    CHECK(con, 'f');
    if (con->json[0] != 'a' || con->json[1] != 'l' || con->json[2] != 's' || con->json[3] != 'e')
        return JSON_PARSE_INVALID_VALUE;
    con->json += 4;
    node->type = JSON_FALSE;
    return JSON_PARSE_SUCCESS;
}

static int json_parse_null(json_context* con, json_node* node) {
    CHECK(con, 'n');
    if (con->json[0] != 'u' || con->json[1] != 'l' || con->json[2] != 'l')
        return JSON_PARSE_INVALID_VALUE;
    con->json += 3;
    node->type = JSON_NULL;
    return JSON_PARSE_SUCCESS;
}

static int json_parse_value(json_context* con,json_node* node)
{
    switch(*con->json)
    {
        case 't':   return json_parse_true(con,node);
        case 'f':   return json_parse_false(con,node);
        case 'n':   return json_parse_null(con,node);
        case '\0':  return JSON_PARSE_EXPECT_VALUE;
        default:    return JSON_PARSE_INVALID_VALUE;
    }
}

int json_parse(json_node* node,const char* json)
{
    json_context con;
    int status;
    assert(node!=NULL);
    con.json = json;
    node->type = JSON_NULL;
    json_parse_space(&con);
    if((status=json_parse_value(&con,node))==JSON_PARSE_SUCCESS)
    {
        json_parse_space(&con);
        if(*con.json!='\0')
        {
            status = JSON_PARSE_ROOT_ERROR;
        }
    }
    return status;
}

json_type json_get_type(const json_node* node)
{
    assert(node!=NULL);
    return node->type;
}


