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
namespace log{
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
        std::string getLogLevel(int level){
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
         * @brief 获取当前时间的字符串表示
         * @return 返回格式化的时间字符串
         */
        std::string getTime(){
            std::time_t now_sec=std::time(nullptr);
            auto now=*std::localtime(&now_sec);
            char timestamp[20];
            std::strftime(timestamp,sizeof(timestamp),"%Y/%m/%d-%H:%M:%S",&now);
            std::string now_time=timestamp;
            return now_time;
        }

        /**
         * @brief 将内容写入日志文件
         * @param content 要写入的内容
         */
        void file_write(std::string content){
            std::ofstream logfile(_PATH/"server.log",std::ios::app);
            if(!logfile.is_open()) std::cerr<<"[ERROR] 打开日志失败！"<<std::endl;
            logfile<<content<<std::endl;
            logfile.close();
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
         * @brief 记录指定级别的日志
         * @param level 日志级别 - 默认 INFO
         * @param str 日志内容
         */
        void log(std::string str,LogLevel level = INFO){
            _log_content=getLogLevel(level)+' '+str;
            std::cout<<_log_content<<std::endl;
            if(level==ERROR) std::cerr<<_log_content<<std::endl;
            file_write(_log_content);
        }

        /**
         * @brief 记录带时间戳的指定级别日志
         * @param level 日志级别 - 默认 INFO
         * @param str 日志内容
         */
        void tlog(std::string str,LogLevel level=INFO){
            std::string timestamp=getTime();
            _log_content="["+timestamp+"] "+getLogLevel(level)+' '+str;
            std::cout<<_log_content<<std::endl;
            if(level==ERROR){
                std::cerr<<_log_content<<std::endl;
            }
            file_write(_log_content);
        }
    };
}
#endif // LOGLIB_H