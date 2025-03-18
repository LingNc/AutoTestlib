#include "Judge.h"

namespace acm{
    JudgeCode judge(process::Status status,int exit_code){
        // 首先检查是否已经标记为超时
        if(status==process::TIMEOUT){
            return acm::TimeLimitEXceeded;
        }
        else if(WIFEXITED(exit_code)){
            int temp=WEXITSTATUS(exit_code);
            if(temp==0){
                return acm::Waiting;
            }
            else{
                return acm::Waiting;
            }
        }
        else if(WIFSIGNALED(exit_code)){
            int signal=WTERMSIG(exit_code);
            switch(signal){
            case SIGSEGV:
                return acm::RuntimeError;
                // 修复逻辑运算符错误，使用单独的case
            case SIGABRT:
                return acm::MemoryLimitExceeded;
            case SIGKILL:
                // 如果已经标记为TIMEOUT，则返回超时错误
                if(status==process::TIMEOUT){
                    return acm::TimeLimitEXceeded;
                }
                return acm::MemoryLimitExceeded;
            case SIGFPE:
                return acm::FloatingPointError;
            default:
                return acm::RuntimeError;
            }
        }
        else{
            return acm::RuntimeError;
        }
    }
}

