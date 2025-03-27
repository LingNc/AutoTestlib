#ifndef AUTO_OPEN_H
#define AUTO_OPEN_H

#include <string>
#include <vector>
#include <optional>
#include <variant>
#include "json.hpp"

// 为std::optional添加JSON序列化支持
namespace nlohmann{
    template <typename T>
    struct adl_serializer<std::optional<T>>{
        static void to_json(json &j,const std::optional<T> &opt){
            if(opt.has_value()){
                j=opt.value();
            }
            else{
                j=nullptr;
            }
        }

        static void from_json(const json &j,std::optional<T> &opt){
            if(j.is_null()){
                opt=std::nullopt;
            }
            else{
                opt=j.get<T>();
            }
        }
    };

    // 为 variant 类型添加通用的 JSON 序列化支持
    template <typename... Ts>
    struct adl_serializer<std::variant<Ts...>>{
        static void to_json(json &j,const std::variant<Ts...> &v){
            std::visit([&j](const auto &value){ j=value; },v);
        }

        static void from_json(const json &j,std::variant<Ts...> &v){
            // 为 RequestMessage 提供特殊处理
            if constexpr(std::is_same_v<std::variant<Ts...>,ns::RequestMessage>){
                if(j.contains("role")){
                    std::string role=j["role"];
                    if(role=="system"){
                        v=j.get<ns::SystemMessage>();
                    }
                    else if(role=="user"){
                        v=j.get<ns::UserMessage>();
                    }
                    else if(role=="assistant"){
                        v=j.get<ns::AssistantMessage>();
                    }
                    else if(role=="tool"){
                        v=j.get<ns::ToolMessage>();
                    }
                }
            }
            // 为 ToolChoice variant 提供处理
            else if constexpr(std::is_same_v<std::variant<Ts...>,std::variant<std::string,ns::ToolChoice>>){
                if(j.is_string()){
                    v=j.get<std::string>();
                }
                else{
                    v=j.get<ns::ToolChoice>();
                }
            }
            // 其他 variant 类型的处理可以在这里添加
        }
    };
}

namespace ns{
    using std::string;
    struct TopLogprob{
        string token;
        double logprob;
        std::vector<uint8_t> bytes;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(TopLogprob,token,logprob,bytes)

        struct ContentLogprob{
        string token;
        double logprob;
        std::vector<uint8_t> bytes;
        std::vector<TopLogprob> top_logprobs;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ContentLogprob,token,logprob,bytes,top_logprobs)

        struct Logprobs{
        std::vector<ContentLogprob> content;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Logprobs,content)

        struct FunctionCall{
        string name;
        string arguments;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(FunctionCall,name,arguments)

        struct ToolCall{
        string id;
        string type;
        FunctionCall function;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ToolCall,id,type,function)

        struct Message{
        string content;
        string reasoning_content="";
        std::vector<ToolCall> tool_calls;
        string role;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Message,content,reasoning_content,tool_calls,role)

        struct Choice{
        string finish_reason;
        int index;
        Message message;
        std::optional<Logprobs> logprobs;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Choice,finish_reason,index,message,logprobs)

        struct CompletionTokensDetails{
        int reasoning_tokens;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(CompletionTokensDetails,reasoning_tokens)

        struct Usage{
        int completion_tokens;
        int prompt_tokens;
        int prompt_cache_hit_tokens;
        int prompt_cache_miss_tokens;
        int total_tokens;
        CompletionTokensDetails completion_tokens_details;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Usage,completion_tokens,prompt_tokens,prompt_cache_hit_tokens,prompt_cache_miss_tokens,total_tokens,completion_tokens_details)

        struct OpenAIResponse{
        string id;
        std::vector<Choice> choices;
        long long created;
        string model;
        string system_fingerprint;
        string object;
        Usage usage;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(OpenAIResponse,id,choices,created,model,system_fingerprint,object,usage)

        struct Parameter{
        string type;
        std::map<string,nlohmann::json> properties;
        std::vector<string> required;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Parameter,type,properties,required)

        struct Function{
        string name;
        std::optional<string> description;
        std::optional<Parameter> parameters;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Function,name,description,parameters)

        struct Tool{
        string type;  // 目前仅支持 "function"
        Function function;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Tool,type,function)

        // 为不同角色的消息创建专用结构体
        struct SystemMessage{
        string role="system";
        string content;
        std::optional<string> name;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SystemMessage,role,content,name)

        struct UserMessage{
        string role="user";
        string content;
        std::optional<string> name;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(UserMessage,role,content,name)

        struct AssistantMessage{
        string role="assistant";
        std::optional<string> content;
        std::optional<string> name;
        std::optional<bool> prefix;
        std::optional<string> reasoning_content;
        std::optional<std::vector<ToolCall>> tool_calls;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(AssistantMessage,role,content,name,prefix,reasoning_content,tool_calls)

        struct ToolMessage{
        string role="tool";
        string content;
        std::optional<string> name;
        string tool_call_id;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ToolMessage,role,content,name,tool_call_id)

        // 使用 variant 替代之前的 RequestMessage
        using RequestMessage=std::variant<SystemMessage,UserMessage,AssistantMessage,ToolMessage>;

    struct ResponseFormat{
        string type;  // "text" 或 "json_object"
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ResponseFormat,type)

        struct StreamOptions{
        std::optional<bool> include_usage;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(StreamOptions,include_usage)

        struct ToolChoice{
        std::optional<string> type;
        std::optional<Function> function;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ToolChoice,type,function)

        struct OpenAIRequest{
        std::vector<RequestMessage> messages; // 必需字段
        string model;                        // 必需字段
        std::optional<double> frequency_penalty;
        std::optional<int> max_tokens;
        std::optional<double> presence_penalty;
        std::optional<ResponseFormat> response_format;
        std::optional<std::vector<string>> stop;
        std::optional<bool> stream;
        std::optional<StreamOptions> stream_options;
        std::optional<double> temperature;
        std::optional<double> top_p;
        std::optional<std::vector<Tool>> tools;
        std::optional<std::variant<string,ToolChoice>> tool_choice;
        std::optional<bool> logprobs;
        std::optional<int> top_logprobs;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(OpenAIRequest,messages,model,frequency_penalty,max_tokens,presence_penalty,response_format,stop,stream,stream_options,temperature,top_p,tools,tool_choice,logprobs,top_logprobs)

} // namespace ns

#endif // AUTO_OPEN_H
