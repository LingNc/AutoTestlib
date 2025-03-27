#include "AutoOpen.h"
#include "json.hpp"

using json=nlohmann::json;
using std::string;

namespace AutoTestlib{
    OpenAIResponse parseOpenAIResponse(const string &jsonString){
        try{
            // 使用新添加的宏定义功能，直接将 JSON 解析为结构体
            return json::parse(jsonString).get<OpenAIResponse>();
        }
        catch(const json::exception &e){
            // JSON 解析错误处理，返回空响应
            return OpenAIResponse{};
        }
    }

} // namespace AutoTestlib
