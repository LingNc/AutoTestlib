#include "sysapi.h"

namespace process{
    // 设置阻塞模式
    void System::set_blocked(bool isblocked){
        _isBlocked=isblocked;
    }
    // 是否阻塞开始
    void System::start_blocked(Handle _pipe){
        if(!_isBlocked){
            _flags=fcntl(_pipe,F_GETFL,0);
            fcntl(_pipe,F_SETFL,_flags|O_NONBLOCK);
        }
    }
    // 是否阻塞结束
    void System::close_blocked(Handle _pipe){
        if(!_isBlocked){
            fcntl(_pipe,F_SETFL,_flags);
        }
    }
    // 设置非阻塞的读取超时时间
    void System::set_ptime(int pipeTime){
        _pipeTime=pipeTime;
    }
}
