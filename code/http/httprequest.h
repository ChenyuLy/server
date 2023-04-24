/*
 * @Author       : mark
 * @Date         : 2020-06-25
 * @copyleft Apache 2.0
 */ 
#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>        //正则表达式库
#include <errno.h>     
#include <mysql/mysql.h>  //mysql

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"
#include "../pool/sqlconnRAII.h"

class HttpRequest {                 //用于处理受到的请求
public:
    enum PARSE_STATE {              //定义各个阶段的表示
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,        
    };

    enum HTTP_CODE {                //http标识
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };
    
    HttpRequest() { Init(); }       //使用init初始化
    ~HttpRequest() = default;

    void Init();                    //初始化 之后解析结束重置状态
    bool parse(Buffer& buff);       //解析

    std::string path() const;       //获取私有类值
    std::string& path();
    std::string method() const;
    std::string version() const;
    std::string GetPost(const std::string& key) const;  //？？？
    std::string GetPost(const char* key) const;         //？？？

    bool IsKeepAlive() const;

    /* 
    todo 
    void HttpConn::ParseFormData() {}
    void HttpConn::ParseJson() {}
    */

private:
    bool ParseRequestLine_(const std::string& line);        //解析各个部分函数
    void ParseHeader_(const std::string& line);
    void ParseBody_(const std::string& line);

    void ParsePath_();
    void ParsePost_();
    void ParseFromUrlencoded_();

    static bool UserVerify(const std::string& name, const std::string& pwd, bool isLogin);

    PARSE_STATE state_;         //实例化阶段标识
    std::string method_, path_, version_, body_;
    std::unordered_map<std::string, std::string> header_;  //？？？
    std::unordered_map<std::string, std::string> post_;     //？？？

    static const std::unordered_set<std::string> DEFAULT_HTML;                      //默认的参数标签
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    static int ConverHex(char ch);
};


#endif //HTTP_REQUEST_H