#ifndef JUDGESIGN_H
#define JUDGESIGN_H

#include "Self.h"

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
}

#endif