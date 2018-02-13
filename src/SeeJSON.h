#ifndef SEEJSON_H_INCLUDED
#define SEEJSON_H_INCLUDED

/// JSON DataType
typedef enum{
    JSON_NULL,
    JSON_FALSE,
    JSON_TRUE,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
}json_type;

/// JSON Data Structure
typedef struct{
    json_type type;
}json_node;

/// Error Code
enum{
    JSON_PARSE_SUCCESS = 0,
    JSON_PARSE_EXPECT_VALUE,
    JSON_PARSE_INVALID_VALUE,
    JSON_PARSE_ROOT_ERROR
};

/******************************************************************
                            Public APIs
******************************************************************/
/// Finished    ( 2018/2/13 Updated )

int json_parse(json_node* json_node,const char* json_str);

json_type json_get_type(const json_node* json_node);


/*****************************************************************/
/// TODO

char* json_encode(json_node* node);

void json_decode(json_node* node,const char* json_str);



#endif // SEEJSON_H_INCLUDED
