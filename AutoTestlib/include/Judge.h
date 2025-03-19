#ifndef JUDGESIGN_H
#define JUDGESIGN_H

#include "Self.h"
#include "Process.h"

namespace acm{
    enum JudgeCode{
        Waiting,
        Compiling,
        Queuing,
        Accept,
        CompilationError,
        WrongAnswer,
        TimeLimitEXceeded,
        MemoryLimitExceeded,
        OutputLimitExceeded,
        FloatingPointError,
        RuntimeError,
        PresentationError
    };
    JudgeCode judge(process::Status status,int exit_code);
    string f(JudgeCode type);
}

#endif