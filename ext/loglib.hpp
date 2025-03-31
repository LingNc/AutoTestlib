/**
 * @file loglib.hpp
 * @brief 简单的日志库实现
 * @details 提供基础的日志记录功能，支持不同日志级别和时间戳
 * @author AutoTestlib
 * @version 1.0
 * @date 创建于未知日期
 */
#ifndef LOGLIB_H
#define LOGLIB_H

#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <filesystem>

 /**
  * @namespace log
  * @brief 日志功能命名空间
  */
namespace loglib{
    namespace fs=std::filesystem;

    /**
     * @enum LogLevel
     * @brief 日志级别枚举
     * @details 定义了四种日志级别：信息、调试、警告和错误
     */
    enum LogLevel{
        INFO=1,   ///< 信息级别
        DEBUG,    ///< 调试级别
        WARNING,  ///< 警告级别
        ERROR     ///< 错误级别
    };

    /**
     * @enum ColorCode
     * @brief 终端颜色代码枚举
     */
    enum ColorCode{
        RESET=0,
        RED=31,
        GREEN=32,
        YELLOW=33,
        BLUE=34,
        MAGENTA=35,
        CYAN=36,
        WHITE=37
    };

    /**
     * @class Log
     * @brief 日志记录类
     * @details 提供日志记录的基本功能，包括不同级别的日志记录和带时间戳的日志
     */
    class Log{
        // 私有成员
        std::string _log_content;  ///< 日志内容
        fs::path _PATH=fs::current_path();  ///< 日志路径
        std::string _name="server.log";    ///< 日志文件名

        /**
         * @brief 获取日志级别的字符串表示
         * @param level 日志级别枚举值
         * @return 返回对应的日志级别字符串
         */
        std::string get_string(LogLevel level){
            switch(level){
            case INFO:
                return "[INFO]";
                break;
            case DEBUG:
                return "[DEBUG]";
                break;
            case WARNING:
                return "[WARING]";
                break;
            case ERROR:
                return "[ERROR]";
                break;
            default:
                return "[?]";
                break;
            }
        }

        /**
         * @brief 获取对应日志级别的颜色代码
         * @param level 日志级别枚举值
         * @return 返回对应的颜色代码
         */
        ColorCode get_color(LogLevel level){
            switch(level){
            case INFO:
                return GREEN;
                break;
            case DEBUG:
                return BLUE;
                break;
            case WARNING:
                return YELLOW;
                break;
            case ERROR:
                return RED;
                break;
            default:
                return RESET;
                break;
            }
        }

        /**
         * @brief 输出指定颜色的字体
         * @param content 要输出的内容
         * @param color 颜色代码
         * @return 带颜色代码的字符串
         */
        std::string set_color(const std::string &content,LogLevel level){
            return "\033["+std::to_string(get_color(level))+"m"+content+"\033[0m";
        }

        /**
         * @brief 获取当前时间的字符串表示
         * @return 返回格式化的时间字符串
         */
        std::string get_time(){
            std::time_t now_sec=std::time(nullptr);
            auto now=*std::localtime(&now_sec);
            char timestamp[20];
            std::strftime(timestamp,sizeof(timestamp),"%Y/%m/%d-%H:%M:%S",&now);
            std::string now_time=timestamp;
            std::string stringTime="["+now_time+"]";
            return stringTime;
        }
    public:
        /**
         * @brief 构造函数
         * @param logPath 日志文件路径，默认为当前路径
         * @param name 日志文件名，默认为server.log
         */
        Log(fs::path logPath=fs::current_path(),std::string name="server.log")
            :_name(name){
            _PATH=logPath;
            // 检查目录是否存在
            if(!fs::exists(_PATH)){
                fs::create_directories(_PATH);
            }
        }

        /**
         * @brief 设置日志文件名
         * @param name 新的日志文件名
         */
        void set_logName(std::string name){
            _name=name;
        }

        /**
         * @brief 设置日志文件路径
         * @param path 新的日志文件路径
         */
        void set_logPath(fs::path path){
            _PATH=path;
            // 检查目录是否存在
            if(!fs::exists(_PATH)){
                fs::create_directories(_PATH);
            }
        }

        /**
         * @brief 将内容写入日志文件
         * @param content 要写入的内容
         * @param level 日志等级
         */
        void wlog(std::string content){
            std::ofstream log_file(_PATH/_name,std::ios::app);
            if(log_file.is_open()){
                log_file<<content<<std::endl;
            }
            else{
                std::cerr<<set_color("无法打开日志文件！",ERROR)<<std::endl;
            }
        }

        /**
         * @brief 记录指定级别的日志
         * @param level 日志级别 - 默认 INFO
         * @param str 日志内容
         */
        void log(std::string str,LogLevel level=INFO){
            _log_content=get_string(level)+' '+str;
            auto &logs=(level==ERROR)?std::cerr:std::cout;
            logs<<set_color(_log_content,level)<<std::endl;
            wlog(_log_content);
        }

        /**
         * @brief 记录带时间戳的指定级别日志
         * @param level 日志级别 - 默认 INFO
         * @param str 日志内容
         */
        void tlog(std::string str,LogLevel level=INFO){
            _log_content=get_time()+" "+get_string(level)+" "+str;
            auto &logs=(level==ERROR)?std::cerr:std::cout;
            logs<<set_color(_log_content,level)<<std::endl;
            wlog(_log_content);
        }
        /**
         * @brief 更新当前这一行的日志，不换行记录
         * @param str 日志内容
         * @param LogLevel 日志级别 - 默认 INFO
         */
        void llog(std::string str,LogLevel level=INFO){
            _log_content=get_string(level)+" "+str;
            auto &logs=(level==ERROR)?std::cerr:std::cout;
            // 输出控制 回到行
            logs<<"\r"<<set_color(_log_content,level)<<std::flush;
        }
        /**
         * @brief 带时间戳的更新当前行的日志，不换行记录
         * @param str 日志内容
         * @param LogLevel 日志级别 - 默认 INFO
         */
        void tllog(std::string str,LogLevel level=INFO){
            _log_content=get_time()+" "+get_string(level)+" "+str;
            auto &logs=(level==ERROR)?std::cerr:std::cout;
            logs<<"\r"<<set_color(_log_content,level)<<std::flush;
        }
        /**
         * @brief 将上一条日志记录到文件
         */
        void fllog(){
            wlog(_log_content);
        }
        /**
         * @brief 向文件中写入日志
         * @param str 日志内容
         * @param level 日志级别 - 默认 INFO
         */
        void flog(std::string str,LogLevel level=INFO){
            _log_content=get_string(level)+' '+str;
            wlog(_log_content);
        }
        /**
         * @brief 向文件中写入带时间戳的日志
         * @param str 日志内容
         * @param level 日志级别 - 默认 INFO
         */
        void tflog(std::string str,LogLevel level=INFO){
            _log_content=get_time()+" "+get_string(level)+" "+str;
            wlog(_log_content);
        }
    };
}
#endif // LOGLIB_H