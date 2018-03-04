# SeeJson
A Simple JSON Parser developed by Pure C 

(Under Development)

SeeJson is a json tool library realized with C language (C89 standard), including analyzer and builder to solve standard JSON string, supporting cross-platform and cross-compiler; the project adopts TDD for dsign including 20 APIs which can satisfy the basic use demand of JSON.

SeeJson是用纯C语言(C89标准)实现的一个json工具库，包含能够处理标准JSON字符串的解析器/生成器，支持跨平台和跨编译器；其中共设计了20个APIs，它们可以满足基本的JSON使用需求

**技术要点 Keypoint：**<br>
* 纯C语言 (C89)
* 使用函数指针实现结构体成员方法，模拟类似class的效果，增强易用性(OOC 面向对象C语言)
* 递归下降分析法
* Test-Driven Development(测试驱动开发)
* 防御性编程
* 跨平台
<hr>

#### 使用说明 ####
(占位待续)

**(0) 导入方法**

导入SeeJSON的方法主要有两种：
* 直接引入.c和.h
* 编译为链接库

**(1) JSON字符串解码为JSON节点**
```c
    const char* json = "{\"name\":\"Amy\"}"; /* 创建JSON字符串 */
    
    json_node node; /* 创建JSON初始节点 */
    
    json_init(&node); /* 初始化 */
    
    json_decode(&node,json); /* 将字符串转换为JSON_NODE结构 */

    printf("name:%s\n",node.getValue(node,"name")); /* 使用getValue成员方法获取键"name"对应的值 */
```
**运行效果:**
```shell
name:Amy
```

**(2) JSON节点编码为JSON字符串**
```c
/* 本例使用的节点是上面编码的那一个node */

size_t length; /* 用于记录生成的字符串的长度 */

char* str = json_encode(&node,&length); /* 解码 */

printf("string:%s\n",str); /* 输出JSON字符串 */

printf("length:%d\n",length); /* 输出字符串长度 */
```
**运行效果:**
```shell
string:{"name":"Amy"}
length:14
```
**更复杂的应用**

有如下JSON文档，名字为"city.json"
```json
{
    "name": "北京市",
    "city": {
        "name": "城区",
        "area": [
          "东城区",
          "海淀区",
          "朝阳区",
          "其他"
        ]
    }
}
```
使用read_json_from_file(path)读取这个文件，可以直接返回一个json_node结构，而无需再手动设置
```c
json_node node = read_json_from_file("city.json");
```
按照JSON文档的层级关系分别调用getValue()，getValue的返回值是空类型指针，必须自己按照对应的数据类型进行类型转换，以后(可能)会实现一个自动类型转换功能。
```c
printf("name1:%s\n",node.getValue(node,"name")); /* 获取第一层的name属性，也就是"北京市"这个字符串 */

/* 获取内层的city属性，这个属性对应的值被解释为一个新的json_object */
/* 所以要再声明一个json_node节点city来存储它，不要忘记转换类型 */
json_node city = *(json_node*)node.getValue(node,"city"); 

/* 从内层节点中取得第二个name属性，"城区"这个字符串 */
printf("name2:%s\n",city.getValue(city,"name"));

/* 获取area属性，SeeJSON的数组类型使用json_node作为容器存储 */
json_node *arr = city.getValue(city,"area");

/* 访问数组元素，目前这个方法还不友好，正在想办法 */
printf("area:%s\n",arr[0].value.string.value);
```
**运行效果**
```shell
name1:北京市
name2:城区
area:东城区
```
**(3) 数据结构和支持的数据类型**

SeeJSON内置的数据类型有：布尔型(true,false)、空值(null)、数值型(number)、字符串型(string)、数组(array)、对象(object)
核心数据结构由结构体json_node定义：
```c
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
    void* (*getValue)(json_node this,char* k);
};

struct json_member{
    char* key;
    size_t key_len;
    json_node node;
};

/* 访问器 */
struct json_visitor{
    char* key;
    json_type type;
    void* value;
};
```

**(4) 多种访问方法**
(占位)
* 直接访问
* getValue()
* 访问器访问

**(5) 注意事项**
(占位)
