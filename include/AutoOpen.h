#ifndef AUTO_OPEN_H
#define AUTO_OPEN_H

#include <string>
#include <vector>
#include "json.hpp"

namespace AutoTestlib{
    using std::string;
    struct TopLogprob{
        string token;
        double logprob;
        std::vector<uint8_t> bytes;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TopLogprob,token,logprob,bytes)

        struct ContentLogprob{
        string token;
        double logprob;
        std::vector<uint8_t> bytes;
        std::vector<TopLogprob> top_logprobs;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ContentLogprob,token,logprob,bytes,top_logprobs)

        struct Logprobs{
        std::vector<ContentLogprob> content;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Logprobs,content)

        struct FunctionCall{
        string name;
        string arguments;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FunctionCall,name,arguments)

        struct ToolCall{
        string id;
        string type;
        FunctionCall function;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ToolCall,id,type,function)

        struct Message{
        string content;
        string reasoning_content;
        std::vector<ToolCall> tool_calls;
        string role;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Message,content,reasoning_content,tool_calls,role)

        struct Choice{
        string finish_reason;
        int index;
        Message message;
        Logprobs logprobs;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Choice,finish_reason,index,message,logprobs)

        struct CompletionTokensDetails{
        int reasoning_tokens;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CompletionTokensDetails,reasoning_tokens)

        struct Usage{
        int completion_tokens;
        int prompt_tokens;
        int prompt_cache_hit_tokens;
        int prompt_cache_miss_tokens;
        int total_tokens;
        CompletionTokensDetails completion_tokens_details;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Usage,completion_tokens,prompt_tokens,prompt_cache_hit_tokens,prompt_cache_miss_tokens,total_tokens,completion_tokens_details)

        struct OpenAIResponse{
        string id;
        std::vector<Choice> choices;
        long long created;
        string model;
        string system_fingerprint;
        string object;
        Usage usage;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(OpenAIResponse,id,choices,created,model,system_fingerprint,object,usage)

        // 解析 JSON 字符串为 OpenAIResponse 结构
        OpenAIResponse parseOpenAIResponse(const string &jsonString);

    // 直接从 JSON 字符串创建结构体
    template<typename T>
    T parseFromJson(const string &jsonString){
        try{
            return nlohmann::json::parse(jsonString).get<T>();
        }
        catch(const nlohmann::json::exception &e){
            // 解析失败时返回默认构造的对象
            return T{};
        }
    }

    // 将结构体转换为 JSON 字符串
    template<typename T>
    string toJsonString(const T &obj){
        return nlohmann::json(obj).dump();
    }

} // namespace AutoTestlib

#endif // AUTO_OPEN_H
