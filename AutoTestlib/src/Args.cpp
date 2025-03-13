#include "Args.h"

namespace process{
    // Args类实现
    Args::Args(){}

    Args::Args(const std::vector<string> &arguments): _args(arguments){
        prepare_c_args();
    }

    Args::Args(const string &command){
        parse(command);
        prepare_c_args();
    }


    Args &Args::add(const string &arg){
        _args.push_back(arg);
        prepare_c_args();
        return *this;
    }

    Args &Args::add(const std::vector<string> &arguments){
        _args.insert(_args.end(),arguments.begin(),arguments.end());
        prepare_c_args();
        return *this;
    }

    Args &Args::set_program_name(const string &name){
        if(_args.empty()){
            _args.push_back(name);
        }
        else{
            _args[0]=name;
        }
        prepare_c_args();
        return *this;
    }

    void Args::clear(){
        _args.clear();
        _c_args.clear();
    }

    size_t Args::size() const{
        return _args.size();
    }

    char **Args::data(){
        prepare_c_args();
        return _c_args.data();
    }

    std::vector<string> Args::get_args() const{
        return _args;
    }

    string Args::get_program_name() const{
        if(!_args.empty()){
            return _args[0];
        }
        return "";
    }

    string &Args::operator[](size_t index){
        return _args[index];
    }

    const string &Args::operator[](size_t index) const{
        return _args[index];
    }

    void Args::prepare_c_args(){
        _c_args.clear();
        for(const auto &arg:_args){
            _c_args.push_back(const_cast<char *>(arg.c_str()));
        }
        _c_args.push_back(nullptr); // execvp需要以nullptr结尾
    }

    // 新增：解析命令行字符串
    Args &Args::parse(const string &command_line){
        enum State{ NORMAL,IN_QUOTE,IN_DQUOTE };
        State state=NORMAL;
        string current_arg;
        bool escaped=false;

        _args.clear(); // 清除旧的参数

        for(char c:command_line){
            if(escaped){
                // 处理转义字符
                current_arg+=c;
                escaped=false;
                continue;
            }

            switch(state){
            case NORMAL:
                if(c=='\\'){
                    escaped=true;
                }
                else if(c=='\''){
                    current_arg+=c;  // 将单引号添加到参数中
                    state=IN_QUOTE;
                }
                else if(c=='\"'){
                    // 不将双引号添加到参数中
                    // current_arg+=c;
                    state=IN_DQUOTE;
                }
                else if(std::isspace(c)){
                    if(!current_arg.empty()){
                        _args.push_back(current_arg);
                        current_arg.clear();
                    }
                }
                else{
                    current_arg+=c;
                }
                break;

            case IN_QUOTE:
                if(c=='\\'){
                    escaped=true;  // 允许在单引号内转义
                }
                else{
                    current_arg+=c;  // 将所有字符添加到参数中，包括单引号
                    if(c=='\''&&!escaped){
                        state=NORMAL;
                    }
                }
                break;

            case IN_DQUOTE:
                if(c=='\\'){
                    escaped=true;  // 允许在双引号内转义
                }
                else{
                    if(c=='\"'&&!escaped){
                        state=NORMAL;
                    }
                    // 在添加之前判断，不包括双引号
                    if(c!='\"') current_arg+=c;
                }
                break;
            }
        }

        // 处理最后一个参数
        if(!current_arg.empty()){
            _args.push_back(current_arg);
        }

        prepare_c_args();
        return *this;
    }

}