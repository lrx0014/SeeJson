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

**(3) 数据结构和支持的数据类型**
(占位)

**(4) 多种访问方法**
(占位)

**(5) 设计思路**
(占位)
