#ifndef SEEJSON_H_INCLUDED
#define SEEJSON_H_INCLUDED

#include <stddef.h> /* size_t */

#ifdef __cplusplus
#define EXPORT extern "C" __declspec (dllexport)
#else
#define EXPORT __declspec (dllexport)
#endif

int isGetErr = 0;

/* JSON DataType */
typedef enum{
    JSON_NULL,
    JSON_FALSE,
    JSON_TRUE,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
}json_type;

typedef struct json_node    json_node;
typedef struct json_member  json_member;
typedef struct json_visitor json_visitor;

/* JSON Data Structure */
struct json_node{
    /* Value of JSON Node */
    union{
        /* Object */
        struct{ json_member* member; size_t size; }object;
        /* Array */
        struct{ json_node* element; size_t size; }array;
        /* String */
        struct{ char* value; size_t length; }string;
        /* Number */
        double number;
    }value;
    /* Type of JSON Node */
    json_type type;
    /* Visit JSON Structure */
    const json_node* (*getValue)(const json_node* this,const char* k);
};

struct json_member{
    char* key;
    size_t key_len;
    json_node node;
};

struct json_visitor{
    /* */
};

/* Error Code */
enum{
    /* 0*/JSON_PARSE_SUCCESS = 0,
    /* 1*/JSON_PARSE_EXPECT_VALUE,
    /* 2*/JSON_PARSE_INVALID_VALUE,
    /* 3*/JSON_PARSE_ROOT_ERROR,
    /* 4*/JSON_PARSE_NUMBER_OVERFLOW,
    /* 5*/JSON_PARSE_NO_QUOTATION_ERROR,
    /* 6*/JSON_PARSE_INVALID_STRING_ESCAPE,
    /* 7*/JSON_PARSE_INVALID_STRING,
    /* 8*/JSON_PARSE_INVALID_UNICODE,
    /* 9*/JSON_PARSE_INVALID_UNICODE_SURROGATE,
    /*10*/JSON_PARSE_UNCOMPLETE_ARRAY_FORMAT,
    /*11*/JSON_PARSE_KEY_NOTFOUND,
    /*12*/JSON_PARSE_MISS_COLON,
    /*13*/JSON_PARSE_UNCOMPLETE_OBJECT_FORMAT,
    /*14*/JSON_GET_STRING_ERROR,
    /*15*/JSON_GET_NUMBER_ERROR,
    /*16*/JSON_GET_BOOLEAN_ERROR,
    /*17*/JSON_GET_NULL_ERROR,
    /*18*/JSON_GET_ARRAY_ERROR,
    /*19*/JSON_GET_OBJECT_ERROR
}json_errcode;

/*
char* err_code[] = {
    "JSON_PARSE_SUCCESS",
    "JSON_PARSE_EXPECT_VALUE",
    "JSON_PARSE_INVALID_VALUE",
    "JSON_PARSE_ROOT_ERROR",
    "JSON_PARSE_NUMBER_OVERFLOW",
    "JSON_PARSE_NO_QUOTATION_ERROR",
    "JSON_PARSE_INVALID_STRING_ESCAPE",
    "JSON_PARSE_INVALID_STRING",
    "JSON_PARSE_INVALID_UNICODE",
    "JSON_PARSE_INVALID_UNICODE_SURROGATE",
    "JSON_PARSE_UNCOMPLETE_ARRAY_FORMAT",
    "JSON_PARSE_KEY_NOTFOUND",
    "JSON_PARSE_MISS_COLON",
    "JSON_PARSE_UNCOMPLETE_OBJECT_FORMAT",
    "JSON_GET_STRING_ERROR",
    "JSON_GET_NUMBER_ERROR",
    "JSON_GET_BOOLEAN_ERROR",
    "JSON_GET_NULL_ERROR",
    "JSON_GET_ARRAY_ERROR",
    "JSON_GET_OBJECT_ERROR"
};
*/

/******************************************************************
                            Public APIs
******************************************************************/
/*               Finished    ( 2018/2/22 Updated )               */

EXPORT void        SeeJSON_Version                 (void);

EXPORT void        json_init                       (json_node* node);

EXPORT void        json_free                       (json_node* node);

EXPORT void        json_set_null                   (json_node* node);

EXPORT int         json_get_bool                   (const json_node* node);

EXPORT void        json_set_bool                   (json_node* node,int val);

EXPORT double      json_get_number                 (const json_node* node);

EXPORT void        json_set_number                 (json_node* node,double val);

EXPORT const char* json_get_string                 (const json_node* node);

EXPORT size_t      json_get_string_length          (const json_node* node);

EXPORT void        json_set_string                 (json_node* node,const char* str,size_t len);

EXPORT size_t      json_get_array_size             (const json_node* node);

EXPORT json_node*  json_get_array_element_by_index (const json_node* node,size_t index);

EXPORT size_t      json_get_object_size            (const json_node* node);

EXPORT const char* json_get_object_key_by_index    (const json_node* node,size_t index);

EXPORT size_t      json_get_object_key_len_by_index(const json_node* node,size_t index);

EXPORT json_node*  json_get_object_value_by_index  (const json_node* node,size_t index);

EXPORT int         json_parse                      (json_node* json_node,const char* json_str);
EXPORT int         json_decode                     (json_node* node,const char* json_str);

EXPORT char*       json_encode                     (const json_node* node,size_t* length);

EXPORT json_type   json_get_type                   (const json_node* json_node);

EXPORT const char* read_string_from_file           (char* path);

EXPORT json_node   read_json_from_file             (char* path);



/******************************************************************
                            TODOs
******************************************************************/

EXPORT json_visitor      see_json   (const json_node* node,const char* key);

EXPORT const char*       getString  (const json_node* node,const char* key);

EXPORT const double      getNumber  (const json_node* node,const char* key);

EXPORT const int         getBoolean (const json_node* node,const char* key);

EXPORT const json_node*  getArray   (const json_node* node,const char* key);

EXPORT const json_node   getObject  (const json_node* node,const char* key);

EXPORT const char*       getNull    (const json_node* node,const char* key);


#endif
