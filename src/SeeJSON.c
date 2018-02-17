/// SeeJSON.c

#include "SeeJSON.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <string.h>

#ifndef JSON_PARSE_STACK_INIT_SIZE
#define JSON_PARSE_STACK_INIT_SIZE 256
#endif // JSON_PARSE_STACK_INIT_SIZE

#define bool  int
#define true  1
#define false 0

#define CHECK(context,ch)   \
            do{ \
                assert(*context->json==(ch));   \
                context->json++;    \
            }while(0)

#define PUTC(c, ch)         do { *(char*)json_context_push(c, sizeof(char)) = (ch); } while(0)


/// JSON String
typedef struct{
    const char* json;
    char* stack;
    size_t size,top;
}json_context;

/* Forward Declaration */
static int json_parse_value(json_context* con,json_node* node);

static bool isDigit(const char ch)
{
    if(ch>='0' && ch<='9')
    {
        return true;
    }
    return false;
}

static bool isOneToNine(const char ch)
{
    if(ch>='1' && ch<='9')
    {
        return true;
    }
    return false;
}

static void* json_context_push(json_context* con,size_t size)
{
    void *ret;
    assert(size>0);
    if(con->top + size >= con->size)
    {
        if(con->size==0)
        {
            con->size = JSON_PARSE_STACK_INIT_SIZE;
        }
        while(con->top + size >= con->size)
        {
            con->size += con->size >> 1;
        }
        con->stack = (char*)realloc(con->stack,con->size);
    }
    ret = con->stack + con->top;
    con->top += size;
    return ret;
}

static void* json_context_pop(json_context* con,size_t size)
{
    assert(con->top >= size);
    return con->stack + (con->top -= size);
}

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

static int json_parse_number(json_context* con,json_node* node)
{
    const char* p = con->json;
    if(*p=='-') p++;
    if(*p=='0') p++;
    else{
        if(!isOneToNine(*p))
        {
            return JSON_PARSE_INVALID_VALUE;
        }
        for(p++;isDigit(*p);p++);
    }
    if(*p=='.')
    {
        p++;
        if(!isDigit(*p))
        {
            return JSON_PARSE_INVALID_VALUE;
        }
        for(p++;isDigit(*p);p++);
    }
    if(*p=='e' || *p=='E')
    {
        p++;
        if(*p=='+' || *p=='-')
        {
            p++;
        }
        if(!isDigit(*p))
        {
            return JSON_PARSE_INVALID_VALUE;
        }
        for(p++;isDigit(*p);p++);
    }
    errno = 0;
    node->value.number = strtod(con->json,NULL);
    if(errno==ERANGE && (node->value.number==HUGE_VAL || node->value.number==-HUGE_VAL))
    {
        return JSON_PARSE_NUMBER_OVERFLOW;
    }
    node->type = JSON_NUMBER;
    con->json = p;
    return JSON_PARSE_SUCCESS;
}

static const char* json_parse_hex4(const char* p,unsigned* u)
{
    *u = 0;
    int i;
    for(i=0;i<4;i++)
    {
        char ch = *p++;
        *u<<=4;
        if(ch>='0' && ch<='9')
        {
            *u |= ch - '0';
        }
        else if(ch>='A' && ch<='F')
        {
            *u |= ch - ('A'-10);
        }
        else if(ch>='a' && ch<='f')
        {
            *u |= ch-('a'-10);
        }
        else{
            return NULL;
        }
    }
    return p;
}

static void json_utf8(json_context* con,unsigned u)
{
    if(u<=0x7F)
    {
        PUTC(con,u & 0xFF);
    }
    else if(u<=0x7FF)
    {
        PUTC(con,0xC0 | ((u>>6)&0xFF));
        PUTC(con,0x80 | ( u   & 0x3F));
    }
    else if(u<=0xFFFF)
    {
        PUTC(con, 0xE0 | ((u >> 12) & 0xFF));
        PUTC(con, 0x80 | ((u >>  6) & 0x3F));
        PUTC(con, 0x80 | ( u        & 0x3F));
    }
    else{
        assert(u <= 0x10FFFF);
        PUTC(con, 0xF0 | ((u >> 18) & 0xFF));
        PUTC(con, 0x80 | ((u >> 12) & 0x3F));
        PUTC(con, 0x80 | ((u >>  6) & 0x3F));
        PUTC(con, 0x80 | ( u        & 0x3F));
    }
}

static int json_parse_string(json_context* con,json_node* node)
{
    size_t head = con->top,len;
    const char* p;
    CHECK(con,'\"');
    p = con->json;
    unsigned u1;
    unsigned u2;
    for(;;)
    {
        char ch = *p++;
        switch(ch)
        {
        case '\"':
            len = con->top - head;
            json_set_string(node,(const char*)json_context_pop(con,len),len);
            con->json = p;
            return JSON_PARSE_SUCCESS;
        case '\\':
            switch (*p++) {
                case '\"': PUTC(con, '\"'); break;
                case '\\': PUTC(con, '\\'); break;
                case '/':  PUTC(con, '/' ); break;
                case 'b':  PUTC(con, '\b'); break;
                case 'f':  PUTC(con, '\f'); break;
                case 'n':  PUTC(con, '\n'); break;
                case 'r':  PUTC(con, '\r'); break;
                case 't':  PUTC(con, '\t'); break;
                case 'u':
                    if(!(p=json_parse_hex4(p,&u1)))
                    {
                        con->top = head;
                        return JSON_PARSE_INVALID_UNICODE;
                    }
                    if(u1>=0xD800 && u1<=0xDBFF)
                    {
                        if(*p++ != '\\')
                        {
                            con->top = head;
                            return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                        }
                        if(*p++ != 'u')
                        {
                            con->top = head;
                            return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                        }
                        if(!(p=json_parse_hex4(p,&u2)))
                        {
                            con->top = head;
                            return JSON_PARSE_INVALID_UNICODE;
                        }
                        if(u2<0xDC00 || u2>0xDFFF)
                        {
                            con->top = head;
                            return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                        }
                        u1 = (((u1-0xD800)<<10)|(u2-0xDC00))+0x10000;
                    }
                    json_utf8(con,u1);
                    break;
                default:
                    con->top = head;
                    return JSON_PARSE_INVALID_STRING_ESCAPE;
            }
            break;
        case '\0':
            con->top = head;
            return JSON_PARSE_NO_QUOTATION_ERROR;
        default:
            if((unsigned char)ch < 0x20)
            {
                con->top = head;
                return JSON_PARSE_INVALID_STRING;
            }
            PUTC(con,ch);
        }
    }
}

static int json_parse_array(json_context* con,json_node* node)
{
    size_t i;
    size_t size = 0;
    int ret;
    CHECK(con,'[');
    json_parse_space(con);
    if(*con->json==']')
    {
        con->json++;
        node->type = JSON_ARRAY;
        node->value.array.size = 0;
        node->value.array.element = NULL;
        return JSON_PARSE_SUCCESS;
    }
    for(;;)
    {
        json_node e;
        json_init(&e);
        if((ret=json_parse_value(con,&e))!=JSON_PARSE_SUCCESS)
        {
            break;
        }
        memcpy(json_context_push(con,sizeof(json_node)),&e,sizeof(json_node));
        size++;
        json_parse_space(con);
        if(*con->json==',')
        {
            con->json++;
            json_parse_space(con);
        }
        else if(*con->json==']')
        {
            con->json++;
            node->type = JSON_ARRAY;
            node->value.array.size = size;
            size *= sizeof(json_node);
            memcpy(node->value.array.element=(json_node*)malloc(size),json_context_pop(con,size),size);
            return JSON_PARSE_SUCCESS;
        }
        else
        {
            ret = JSON_PARSE_UNCOMPLETE_ARRAY_FORMAT;
            break;
        }
    }
    for(i=0;i<size;i++)
    {
        json_free((json_node*)json_context_pop(con,sizeof(json_node)));
    }
    return ret;
}

static int json_parse_value(json_context* con,json_node* node)
{
    switch(*con->json)
    {
        case 't' :  return json_parse_true(con,node);
        case 'f' :  return json_parse_false(con,node);
        case 'n' :  return json_parse_null(con,node);
        case '\0':  return JSON_PARSE_EXPECT_VALUE;
        case '"' :  return json_parse_string(con,node);
        case '[' :  return json_parse_array(con,node);
        default  :  return json_parse_number(con,node);
    }
}

/**********************************************************
                Implements of Public APIs
**********************************************************/

void json_init(json_node* node)
{
    node->type = JSON_NULL;
}

void json_set_null(json_node* node)
{
    json_free(node);
}

int json_parse(json_node* node,const char* json)
{
    json_context con;
    int status;
    assert(node!=NULL);
    con.json = json;
    con.stack = NULL;
    con.size = con.top = 0;
    json_init(node);
    json_parse_space(&con);
    if((status=json_parse_value(&con,node))==JSON_PARSE_SUCCESS)
    {
        json_parse_space(&con);
        if(*con.json!='\0')
        {
            node->type = JSON_NULL;
            status = JSON_PARSE_ROOT_ERROR;
        }
    }
    assert(con.top == 0);
    free(con.stack);
    return status;
}

json_type json_get_type(const json_node* node)
{
    assert(node!=NULL);
    return node->type;
}

double json_get_number(const json_node* node)
{
    assert(node!=NULL && node->type==JSON_NUMBER);
    return node->value.number;
}

void json_set_number(json_node* node,double n)
{
    json_free(node);
    node->value.number = n;
    node->type = JSON_NUMBER;
}

void json_free(json_node* node)
{
    assert(node!=NULL);
    size_t i;
    switch(node->type)
    {
    case JSON_STRING:
        free(node->value.string.value);
        break;

    case JSON_ARRAY:
        for(i=0;i<node->value.array.size;i++)
        {
            json_free(&node->value.array.element[i]);
        }
        free(node->value.array.element);
        break;

    default:break;
    }
    node->type = JSON_NULL;
}

int json_get_bool(const json_node* node)
{
    assert(node!=NULL && (node->type==JSON_TRUE || node->type==JSON_FALSE));
    return node->type == JSON_TRUE;
}

void json_set_bool(json_node* node,int b)
{
    json_free(node);
    node->type = b?JSON_TRUE:JSON_FALSE;
}

const char* json_get_string(const json_node* node)
{
    assert(node!=NULL && node->type==JSON_STRING);
    return node->value.string.value;
}

size_t json_get_string_length(const json_node* node)
{
    assert(node!=NULL && node->type==JSON_STRING);
    return node->value.string.length;
}

void json_set_string(json_node* node,const char* str,size_t len)
{
    assert(node!=NULL && (str!=NULL || len==0));
    json_free(node);
    node->value.string.value = (char*)malloc(len+1);
    memcpy(node->value.string.value,str,len);
    node->value.string.value[len] = '\0';
    node->value.string.length = len;
    node->type = JSON_STRING;
}

size_t json_get_array_size(const json_node* node)
{
    assert(node!=NULL && node->type==JSON_ARRAY);
    return node->value.array.size;
}

json_node* json_get_array_element_by_index(const json_node* node,size_t index)
{
    assert(node!=NULL && node->type==JSON_ARRAY);
    assert(index<node->value.array.size);
    return &node->value.array.element[index];
}


