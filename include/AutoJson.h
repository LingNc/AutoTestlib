#ifndef AUTO_OPEN_H
#define AUTO_OPEN_H

#include <string>
#include <vector>
#include <optional>
#include <variant>
#include "json.hpp"
namespace nlohmann{
    // 转换 optional<T>
    template <typename T>
    struct adl_serializer<std::optional<T>>{
        // 序列化 optional 到 JSON
        static void to_json(json &j,const std::optional<T> &opt){
            if(opt) j=opt.value();
            else j=nullptr;
        }
        // 反序列化 JSON 到 optional
        static void from_json(const json &j,std::optional<T> &opt){
            if(j.is_null()) opt=std::nullopt;
            else opt=j.get<T>();
        }
    };
    // 转换 variant<T...>
    template <typename ...Ts>
    struct adl_serializer<std::variant<Ts...>>{
        // 序列化 variant 到 JSON
        static void to_json(json &j,std::variant<Ts...> const &v){
            std::visit([&j](auto &&value){
                j=std::forward<decltype(value)>(value);
                },v);
        }

        // 反序列化 JSON 到 variant
        static void from_json(json const &j,std::variant<Ts...> &v){
            bool converted=false;
            // 使用折叠表达式尝试将 json 转换为每个可能的类型
            (try_convert<Ts>(j,v,converted)||...);

            // 如果无法转换，抛出异常
            if(!converted){
                // 错误修复：添加第三个参数 j（上下文）
                throw detail::type_error::create(302,
                    "无法将JSON值转换为variant: "+std::string(j.type_name()),&j);
                // 不抛出异常，而是使用第一个类型的默认值
                // v = std::variant<Ts...>{std::in_place_type<typename std::tuple_element<0, std::tuple<Ts...>>::type>};
            }
        }

    private:
        // 尝试将 JSON 转换为特定类型并存入 variant
        template<typename T>
        static bool try_convert(const json &j,std::variant<Ts...> &v,bool &converted){
            if(converted) return true; // 已经成功转换，直接返回

            try{
                v=j.get<T>();
                converted=true;
                return true;
            }
            catch(...){
                // 此类型转换失败，返回 false 继续尝试其他类型
                return false;
            }
        }
    };
// template <>
    // struct adl_serializer<ns::RequestMessage>{
    //     static void to_json(json &j,const ns::RequestMessage &msg){
    //         std::visit([&j](auto &&value){
    //             j=std::forward<decltype(value)>(value);
    //             },msg);
    //     }

    //     static void from_json(const json &j,ns::RequestMessage &msg){
    //         if(!j.contains("role")){
    //             throw detail::type_error::create(302,"缺少必须的 'role' 字段无法识别消息类型",&j);
    //         }

    //         const std::string role=j["role"];

    //         if(role=="system"){
    //             msg=j.get<ns::SystemMessage>();
    //         }
    //         else if(role=="user"){
    //             msg=j.get<ns::UserMessage>();
    //         }
    //         else if(role=="assistant"){
    //             msg=j.get<ns::AssistantMessage>();
    //         }
    //         else if(role=="tool"){
    //             msg=j.get<ns::ToolMessage>();
    //         }
    //         else{
    //             throw detail::type_error::create(302,
    //                 "未知的角色类型: "+role,&j);
    //         }
    //     }
    // };
    // template <>
    // struct adl_serializer<std::variant<std::string,ns::ToolChoice>>{
    //     static void to_json(json &j,const std::variant<std::string,ns::ToolChoice> &v){
    //         std::visit([&j](auto &&value){
    //             j=std::forward<decltype(value)>(value);
    //             },v);
    //     }

    //     static void from_json(const json &j,std::variant<std::string,ns::ToolChoice> &v){
    //         if(j.is_string()){
    //             v=j.get<std::string>();
    //         }
    //         else if(j.is_object()){
    //             v=j.get<ns::ToolChoice>();
    //         }
    //         else{
    //             throw detail::type_error::create(302,
    //                 "无法将JSON值类型 "+std::string(j.type_name())+" 转换为 string 或 ToolChoice",&j);
    //         }
    //     }
    // };
    // template <>
    // struct adl_serializer<ns::RequestMessage>{
    //     static void to_json(json &j,const ns::RequestMessage &msg){
    //         std::visit([&j](auto &&value){
    //             j=std::forward<decltype(value)>(value);
    //             },msg);
    //     }

    //     static void from_json(const json &j,ns::RequestMessage &msg){
    //         if(!j.contains("role")){
    //             throw detail::type_error::create(302,"缺少必须的 'role' 字段无法识别消息类型",&j);
    //         }

    //         const std::string role=j["role"];

    //         if(role=="system"){
    //             msg=j.get<ns::SystemMessage>();
    //         }
    //         else if(role=="user"){
    //             msg=j.get<ns::UserMessage>();
    //         }
    //         else if(role=="assistant"){
    //             msg=j.get<ns::AssistantMessage>();
    //         }
    //         else if(role=="tool"){
    //             msg=j.get<ns::ToolMessage>();
    //         }
    //         else{
    //             throw detail::type_error::create(302,
    //                 "未知的角色类型: "+role,&j);
    //         }
    //     }
    // };
    // template <>
    // struct adl_serializer<std::variant<std::string,ns::ToolChoice>>{
    //     static void to_json(json &j,const std::variant<std::string,ns::ToolChoice> &v){
    //         std::visit([&j](auto &&value){
    //             j=std::forward<decltype(value)>(value);
    //             },v);
    //     }

    //     static void from_json(const json &j,std::variant<std::string,ns::ToolChoice> &v){
    //         if(j.is_string()){
    //             v=j.get<std::string>();
    //         }
    //         else if(j.is_object()){
    //             v=j.get<ns::ToolChoice>();
    //         }
    //         else{
    //             throw detail::type_error::create(302,
    //                 "无法将JSON值类型 "+std::string(j.type_name())+" 转换为 string 或 ToolChoice",&j);
    //         }
    //     }
    // };
}

namespace ns{
    using std::string;
    // 解析器
    struct TopLogprob{
        string token;
        double logprob;
        std::vector<uint8_t> bytes;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(TopLogprob,token,logprob,bytes)
        // 解析器
        struct ContentLogprob{
        string token;
        double logprob;
        std::vector<uint8_t> bytes;
        std::vector<TopLogprob> top_logprobs;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ContentLogprob,token,logprob,bytes,top_logprobs)
        // 解析器
        struct Logprobs{
        std::vector<ContentLogprob> content;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Logprobs,content)
        // 函数调用
        struct FunctionCall{
        string name;
        string arguments;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(FunctionCall,name,arguments)
        // 工具调用
        struct ToolCall{
        string id;
        string type;
        FunctionCall function;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ToolCall,id,type,function)
        // 助手消息
        struct Message{
        string content;
        string reasoning_content="";
        std::vector<ToolCall> tool_calls;
        string role;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Message,content,reasoning_content,tool_calls,role)
        // 助手消息
        struct Choice{
        string finish_reason;
        int index;
        Message message;
        std::optional<Logprobs> logprobs;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Choice,finish_reason,index,message,logprobs)
        // 推理令牌
        struct CompletionTokensDetails{
        int reasoning_tokens;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(CompletionTokensDetails,reasoning_tokens)
        // 使用情况
        struct Usage{
        int completion_tokens;
        int prompt_tokens;
        int prompt_cache_hit_tokens;
        int prompt_cache_miss_tokens;
        int total_tokens;
        CompletionTokensDetails completion_tokens_details;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Usage,completion_tokens,prompt_tokens,prompt_cache_hit_tokens,prompt_cache_miss_tokens,total_tokens,completion_tokens_details)
        // 解析器
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
        // 工具调用
        struct Parameter{
        string type;
        std::map<string,nlohmann::json> properties;
        std::vector<string> required;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Parameter,type,properties,required)
        // 函数
        struct Function{
        string name;
        std::optional<string> description;
        std::optional<Parameter> parameters;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Function,name,description,parameters)
        // 工具
        struct Tool{
        string type;  // 目前仅支持 "function"
        Function function;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Tool,type,function)
        // 系统消息
        struct SystemMessage{
        string role="system";
        string content;
        std::optional<string> name;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SystemMessage,role,content,name)
        // 用户消息
        struct UserMessage{
        string role="user";
        string content;
        std::optional<string> name;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(UserMessage,role,content,name)
        // 助手消息
        struct AssistantMessage{
        string role="assistant";
        std::optional<string> content;
        std::optional<string> name;
        std::optional<bool> prefix;
        std::optional<string> reasoning_content;
        std::optional<std::vector<ToolCall>> tool_calls;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(AssistantMessage,role,content,name,prefix,reasoning_content,tool_calls)
        // 工具消息
        struct ToolMessage{
        string role="tool";
        string content;
        std::optional<string> name;
        string tool_call_id;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ToolMessage,role,content,name,tool_call_id)

        // 使用 variant RequestMessage
        using RequestMessage=std::variant<SystemMessage,UserMessage,AssistantMessage,ToolMessage>;
    // 定义类型转换
    struct ResponseFormat{
        string type;  // "text" 或 "json_object"
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ResponseFormat,type)
        // 流式选项
        struct StreamOptions{
        std::optional<bool> include_usage;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(StreamOptions,include_usage)
        // 工具选择
        struct ToolChoice{
        std::optional<string> type;
        std::optional<Function> function;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ToolChoice,type,function)
        // 请求体
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
