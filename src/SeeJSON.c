/* SeeJSON.c         */
/* SeeJSON @Ryann    */
/* Under Development */
/* 2018/3/16 Updated */

#include "SeeJSON.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <string.h>

#ifndef JSON_PARSE_STACK_INIT_SIZE
#define JSON_PARSE_STACK_INIT_SIZE 256
#endif

#ifndef JSON_ENCODE_INIT_SIZE
#define JSON_ENCODE_INIT_SIZE 256
#endif

#define bool  int
#define true  1
#define false 0

#define CHECK(context,ch)   \
            do{ \
                assert(*context->json==(ch));   \
                context->json++;    \
            }while(0)

#define PUTC(c, ch)         do { *(char*)json_context_push(c, sizeof(char)) = (ch); } while(0)

#define PUTS(c, s, len)     memcpy(json_context_push(c, len), s, len)


/* JSON String */
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
    if(*p=='-')
    {
        p++;
    }
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

static int json_parse_string_raw(json_context* con,char** str,size_t* len)
{
    size_t head = con->top;
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
            *len = con->top - head;
            *str = json_context_pop(con,*len);
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

static int json_parse_string(json_context* con,json_node* node)
{
    int ret;
    char* s;
    size_t len;
    if((ret=json_parse_string_raw(con,&s,&len))==JSON_PARSE_SUCCESS)
    {
        json_set_string(node,s,len);
    }
    return ret;
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

static int json_parse_object(json_context* con,json_node* node)
{
    size_t i,size;
    json_member member;
    int ret;
    CHECK(con,'{');
    json_parse_space(con);
    if(*con->json=='}')
    {
        con->json++;
        node->type = JSON_OBJECT;
        node->value.object.member = 0;
        node->value.object.size = 0;
        return JSON_PARSE_SUCCESS;
    }
    member.key = NULL;
    size = 0;
    for(;;)
    {
        char* str;
        json_init(&member.node);
        if(*con->json!='"')
        {
            ret = JSON_PARSE_KEY_NOTFOUND;
            break;
        }
        if((ret=json_parse_string_raw(con,&str,&member.key_len))!=JSON_PARSE_SUCCESS)
        {
            break;
        }
        memcpy(member.key=(char*)malloc(member.key_len+1),str,member.key_len);
        member.key[member.key_len] = '\0';
        json_parse_space(con);
        if(*con->json!=':')
        {
            ret = JSON_PARSE_MISS_COLON;
            break;
        }
        con->json++;
        json_parse_space(con);
        if((ret=json_parse_value(con,&member.node))!=JSON_PARSE_SUCCESS)
        {
            break;
        }
        memcpy(json_context_push(con,sizeof(json_member)),&member,sizeof(json_member));
        size++;
        member.key = NULL;
        json_parse_space(con);
        if(*con->json==',')
        {
            con->json++;
            json_parse_space(con);
        }
        else if(*con->json=='}')
        {
            size_t s =sizeof(json_member)*size;
            con->json++;
            node->type = JSON_OBJECT;
            node->value.object.size = size;
            memcpy(node->value.object.member=(json_member*)malloc(s),json_context_pop(con,s),s);
            return JSON_PARSE_SUCCESS;
        }
        else{
            ret = JSON_PARSE_UNCOMPLETE_OBJECT_FORMAT;
            break;
        }
    }
    free(member.key);
    for(i=0;i<size;i++)
    {
        json_member* member = (json_member*)json_context_pop(con,sizeof(json_member));
        free(member->key);
        json_free(&member->node);
    }
    node->type = JSON_NULL;
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
        case '{' :  return json_parse_object(con,node);
        default  :  return json_parse_number(con,node);
    }
}

static void json_encode_string(json_context* con,const char* s,size_t len)
{
    static const char hex_digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    size_t i,size;
    char* head,*p;
    assert(s!=NULL);
    head = json_context_push(con,size=len*6+2);
    p = head;
    *p++ = '"';
    for(i=0;i<len;i++)
    {
        unsigned char ch = (unsigned char)s[i];
        switch(ch)
        {
            case '\"': *p++ = '\\'; *p++ = '\"'; break;
            case '\\': *p++ = '\\'; *p++ = '\\'; break;
            case '\b': *p++ = '\\'; *p++ = 'b';  break;
            case '\f': *p++ = '\\'; *p++ = 'f';  break;
            case '\n': *p++ = '\\'; *p++ = 'n';  break;
            case '\r': *p++ = '\\'; *p++ = 'r';  break;
            case '\t': *p++ = '\\'; *p++ = 't';  break;
            default:
                if (ch < 0x20)
                {
                    *p++ = '\\'; *p++ = 'u'; *p++ = '0'; *p++ = '0';
                    *p++ = hex_digits[ch >> 4];
                    *p++ = hex_digits[ch & 15];
                }
                else
                {
                    *p++ = s[i];
                }

        }
    }
    *p++ = '"';
    con->top -= size - (p-head);
}

static void json_encode_value(json_context* con,const json_node* node)
{
    size_t i;
    switch(node->type)
    {
        case JSON_NULL  : PUTS(con,"null",4);  break;
        case JSON_FALSE : PUTS(con,"false",5); break;
        case JSON_TRUE  : PUTS(con,"true",4);  break;
        case JSON_NUMBER: con->top -= 32 - sprintf(json_context_push(con,32),"%.17g",node->value.number); break;
        case JSON_STRING: json_encode_string(con,node->value.string.value,node->value.string.length); break;
        case JSON_ARRAY :
            PUTC(con,'[');
            for(i=0;i<node->value.array.size;i++)
            {
                if(i>0)
                {
                    PUTC(con,',');
                }
                json_encode_value(con,&node->value.array.element[i]);
            }
            PUTC(con,']');
            break;
        case JSON_OBJECT:
            PUTC(con,'{');
            for(i=0;i<node->value.object.size;i++)
            {
                if(i>0)
                {
                    PUTC(con,',');
                }
                json_encode_string(con,node->value.object.member[i].key,node->value.object.member[i].key_len);
                PUTC(con,':');
                json_encode_value(con,&node->value.object.member[i].node);
            }
            PUTC(con,'}');
            break;
        default:
            assert(0 && "INVALID TYPE ERROR...");
    }
}

static const json_node* getValue(const json_node* this,const char* k)
{
    int this_size = this->value.object.size;
    json_node* ret;

    int i;
    for(i=0;i<this_size;i++)
    {
        json_member* cur;
        cur = &(this->value.object.member[i]);
        if(strcmp(cur->key,k)==0)
        {
            ret = &cur->node;
            return ret;
        }
    }
    return NULL;
}

/**********************************************************
                Implements of Public APIs
**********************************************************/

EXPORT void SeeJSON_Version(void)
{
    printf("SeeJSON ( ver-alpha 1.0.0 )  2018-01-30 \n\n");
}

EXPORT void json_init(json_node* node)
{
    node->type = JSON_NULL;
    node->getValue = getValue;
}

EXPORT void json_set_null(json_node* node)
{
    json_free(node);
}

EXPORT int json_parse(json_node* node,const char* json)
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

EXPORT json_type json_get_type(const json_node* node)
{
    assert(node!=NULL);
    return node->type;
}

EXPORT double json_get_number(const json_node* node)
{
    assert(node!=NULL && node->type==JSON_NUMBER);
    return node->value.number;
}

EXPORT void json_set_number(json_node* node,double n)
{
    json_free(node);
    node->value.number = n;
    node->type = JSON_NUMBER;
}

EXPORT void json_free(json_node* node)
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

    case JSON_OBJECT:
        for(i=0;i<node->value.object.size;i++)
        {
            free(node->value.object.member[i].key);
            json_free(&node->value.object.member[i].node);
        }
        free(node->value.object.member);
        break;

    default:break;
    }
    node->type = JSON_NULL;
}

EXPORT int json_get_bool(const json_node* node)
{
    assert(node!=NULL && (node->type==JSON_TRUE || node->type==JSON_FALSE));
    return node->type == JSON_TRUE;
}

EXPORT void json_set_bool(json_node* node,int b)
{
    json_free(node);
    node->type = b?JSON_TRUE:JSON_FALSE;
}

EXPORT const char* json_get_string(const json_node* node)
{
    assert(node!=NULL && node->type==JSON_STRING);
    return node->value.string.value;
}

EXPORT size_t json_get_string_length(const json_node* node)
{
    assert(node!=NULL && node->type==JSON_STRING);
    return node->value.string.length;
}

EXPORT void json_set_string(json_node* node,const char* str,size_t len)
{
    assert(node!=NULL && (str!=NULL || len==0));
    json_free(node);
    node->value.string.value = (char*)malloc(len+1);
    memcpy(node->value.string.value,str,len);
    node->value.string.value[len] = '\0';
    node->value.string.length = len;
    node->type = JSON_STRING;
}

EXPORT size_t json_get_array_size(const json_node* node)
{
    assert(node!=NULL && node->type==JSON_ARRAY);
    return node->value.array.size;
}

EXPORT json_node* json_get_array_element_by_index(const json_node* node,size_t index)
{
    assert(node!=NULL && node->type==JSON_ARRAY);
    assert(index<node->value.array.size);
    return &node->value.array.element[index];
}

EXPORT size_t json_get_object_size(const json_node* node)
{
    assert(node!=NULL && node->type==JSON_OBJECT);
    return node->value.object.size;
}

EXPORT const char* json_get_object_key_by_index(const json_node* node,size_t index)
{
    assert(node!=NULL && node->type==JSON_OBJECT);
    assert(index<node->value.object.size);
    return node->value.object.member[index].key;
}

EXPORT size_t json_get_object_key_len_by_index(const json_node* node,size_t index)
{
    assert(node!=NULL && node->type==JSON_OBJECT);
    assert(index<node->value.object.size);
    return node->value.object.member[index].key_len;
}

EXPORT json_node* json_get_object_value_by_index(const json_node* node,size_t index)
{
    assert(node!=NULL && node->type==JSON_OBJECT);
    assert(index<node->value.object.size);
    return &node->value.object.member[index].node;
}

EXPORT char* json_encode(const json_node* node,size_t* length)
{
    json_context con;
    assert(node!=NULL);
    con.stack = (char*)malloc(con.size=JSON_ENCODE_INIT_SIZE);
    con.top = 0;
    json_encode_value(&con,node);
    if(length)
    {
        *length = con.top;
    }
    PUTC(&con,'\0');
    return con.stack;
}

EXPORT int json_decode(json_node* node,const char* json_str)
{
    return json_parse(node,json_str);
}

EXPORT const char* read_string_from_file(char* path)
{
    assert(path!=NULL);
    FILE *fp;
    char *str;
    char txt[1000];
    int filesize;
    if ((fp=fopen(path,"r"))==NULL)
    {
        printf("Open File:%s Error!!\n",path);
        return NULL;
    }

    fseek(fp,0,SEEK_END);

    filesize = ftell(fp);
    str=(char *)malloc(filesize);
    str[0]=0;

    rewind(fp);
    while((fgets(txt,1000,fp))!=NULL)
    {
        strcat(str,txt);
    }
    fclose(fp);
    return str;
}

EXPORT json_node read_json_from_file(char* path)
{
    assert(path!=NULL);
    const char* str;
    str = read_string_from_file(path);
    json_node node;
    json_init(&node);
    if(str!=NULL)
    {
        json_decode(&node,str);
    }else{
        node.type = JSON_NULL;
    }

    return node;
}

EXPORT const char* getString(const json_node* node,const char* key)
{
    assert(node!=NULL);
    isGetErr = 0;
    if(strcmp(key,"")==0 && node->type==JSON_STRING)
    {
        return node->value.string.value;
    }

    const json_node* temp = node->getValue(node,key);
    if(temp->type != JSON_STRING)
    {
        isGetErr = JSON_GET_STRING_ERROR;
        return "{ERROR:This node does not contain a string!}";
    }

    return temp->value.string.value;
}

EXPORT const double getNumber(const json_node* node,const char* key)
{
    assert(node!=NULL);
    isGetErr = 0;
    if(strcmp(key,"")==0 && node->type==JSON_NUMBER)
    {
        return node->value.number;
    }

    const json_node* temp = node->getValue(node,key);
    if(temp->type != JSON_NUMBER)
    {
        isGetErr = JSON_GET_NUMBER_ERROR;
        return 0;
    }
    return temp->value.number;
}

EXPORT const int getBoolean(const json_node* node,const char* key)
{
    assert(node!=NULL);
    isGetErr = 0;
    if(strcmp(key,"")==0 && (node->type==JSON_TRUE||node->type==JSON_FALSE))
    {
        printf("aaa\n");
        if(node->type==JSON_TRUE)
        {
            return 1;
        }
        if(node->type==JSON_FALSE)
        {
            return 0;
        }
    }
    const json_node* temp = node->getValue(node,key);
    if(temp->type!=JSON_FALSE || temp->type!=JSON_TRUE)
    {
        isGetErr = JSON_GET_BOOLEAN_ERROR;
        return false;
    }
    if(temp->type == JSON_TRUE)
    {
        return 1;
    }
    if(temp->type == JSON_FALSE)
    {
        return 0;
    }
}

EXPORT const char* getNull(const json_node* node,const char* key)
{
    assert(node!=NULL);
    isGetErr = 0;
    if(strcmp(key,"")==0 && node->type==JSON_NULL)
    {
        return "null";
    }
    const json_node* temp = node->getValue(node,key);
    if(temp->type != JSON_NULL)
    {
        isGetErr = JSON_GET_NULL_ERROR;
        return "{ERROR Type!}";
    }
    return "null";
}

EXPORT const json_node getObject(const json_node* node,const char* key)
{
    assert(node!=NULL);
    isGetErr = 0;
    if(strcmp(key,"")==0 && node->type==JSON_OBJECT)
    {
        /* TODO */
    }
    const json_node* temp = node->getValue(node,key);
    if(temp->type != JSON_OBJECT)
    {
        isGetErr = JSON_GET_OBJECT_ERROR;
        return *node;
    }
    return *temp;
}

EXPORT const json_node* getArray(const json_node* node,const char* key)
{
    assert(node!=NULL);
    isGetErr = 0;
    if(strcmp(key,"")==0 && node->type==JSON_ARRAY)
    {
        /* TODO */
    }
    const json_node* temp = node->getValue(node,key);
    if(temp->type != JSON_ARRAY)
    {
        isGetErr = JSON_GET_ARRAY_ERROR;
        return NULL;
    }
    return temp->value.array.element;
}
