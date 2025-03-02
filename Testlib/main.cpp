#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include "lib/ext/boost/include/boost/process.hpp"
#include <cstdlib>
int main(){
    pid_t pid=fork();
    if(pid==0){
        execl("/bin/g++","g++","./Test_Code/1.cpp","-o","./Test_Code/1",NULL);
        perror("编译失败");
        exit(1);
    }
    else{
        int status;
        waitpid(pid,&status,0);
        if(WIFEXITED(status)&&WEXITSTATUS(status)==0){
            std::cout<<"编译成功"<<std::endl;
            pid=fork();
            if(pid==0){
                execl("./Test_Code/1","./Test_Code/1",NULL);
                std::cerr<<"运行失败"<<std::endl;
                exit(1);
            }
            else{
                wait(NULL);
                std::cout<<"运行成功"<<std::endl;
            }
        }
        else{
            std::cerr<<"编译失败"<<std::endl;
        }

    }
    return 0;
}