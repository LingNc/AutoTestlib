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
}

namespace ns{
    using std::string;

    //===========================
    // OpenAI API 相关结构体
    //===========================

    // 请求相关
    struct Request{
        // 消息相关结构体
        struct MultiMessage{
            struct System{
                string role="system";
                string content="";
                std::optional<string> name;
            };

            struct User{
                string role="user";
                string content="";
                std::optional<string> name;
            };

            struct Assistant{
                string role="assistant";
                string content="";
                std::optional<string> name;
                std::optional<bool> prefix;
                std::optional<string> reasoning_content;
                // std::optional<std::vector<ToolCall>> tool_calls;
            };

            struct Tool{
                string role="tool";
                string content="";
                std::optional<string> name;
                string tool_call_id;
            };
            using Message=std::variant<System,User,Assistant,Tool>;
        };
        // 使用variant定义消息类型
        using Message=MultiMessage::Message;
        struct Format{
            string type;  // "text" 或 "json_object"
        };
        struct StreamOptions{
            std::optional<bool> include_usage;
        };
        struct Tool{
            struct Function{
                // 工具和函数相关
                struct Parameter{
                    string type;
                    std::map<string,nlohmann::json> properties;
                    std::vector<string> required;
                };
                string name;
                string description;
                Parameter parameters;
            };
            string type;  // 目前仅支持 "function"
            Function function;
        };
        struct ToolChoice{
            struct Function{
                string name;
            };
            Function function;
            string type;
        };

        std::vector<Message> messages; // 必需字段
        string model;                        // 必需字段
        std::optional<double> frequency_penalty;
        std::optional<int> max_tokens;
        std::optional<double> presence_penalty;
        std::optional<Format> response_format;
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

    // 响应相关
    struct Response{
        struct Usage{
            struct CompletionDetails{
                int reasoning_tokens;
            };

            int completion_tokens;
            int prompt_tokens;
            int prompt_cache_hit_tokens;
            int prompt_cache_miss_tokens;
            int total_tokens;
            CompletionDetails completion_tokens_details;
        };
        struct Message{
            struct ToolCall{
                struct FunctionCall{
                    string name;
                    string arguments;
                };
                string id;
                string type;
                FunctionCall function;
            };
            string content="";
            string reasoning_content="";
            std::vector<ToolCall> tool_calls;
            string role;
        };
        // 日志和Token相关
        struct LogProb{
            struct Top{
                string token;
                double logprob;
                std::vector<uint8_t> bytes;
            };

            struct Content{
                string token;
                double logprob;
                std::vector<uint8_t> bytes;
                std::vector<Top> top_logprobs;
            };

            std::vector<Content> content;
        };
        struct Choice{
            string finish_reason;
            int index;
            Message message;
            std::optional<LogProb> logprobs;
        };
        using ToolCall=Message::ToolCall;
        string id;
        std::vector<Choice> choices;
        long long created;
        string model;
        string system_fingerprint;
        string object;
        Usage usage;
    };

    // 类型宏定义 - API相关
    // LogProb 相关
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Response::LogProb::Top, token, logprob, bytes)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Response::LogProb::Content, token, logprob, bytes, top_logprobs)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Response::LogProb, content)

    // 工具和函数相关
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Request::Tool::Function::Parameter, type, properties, required)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Request::Tool::Function, name, description, parameters)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Request::Tool, type, function)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Response::Message::ToolCall::FunctionCall, name, arguments)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Response::Message::ToolCall, id, type, function)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Request::ToolChoice::Function, name)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Request::ToolChoice, type, function)

    // 消息相关 - 更新为MultiMessage内的结构
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Request::MultiMessage::System, role, content, name)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Request::MultiMessage::User, role, content, name)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Request::MultiMessage::Assistant, role, content, name, prefix, reasoning_content)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Request::MultiMessage::Tool, role, content, name, tool_call_id)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Response::Message, content, reasoning_content, tool_calls, role)

    // 请求和响应格式相关
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Request::Format, type)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Request::StreamOptions, include_usage)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Response::Usage::CompletionDetails, reasoning_tokens)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Response::Usage, completion_tokens, prompt_tokens, prompt_cache_hit_tokens, prompt_cache_miss_tokens, total_tokens, completion_tokens_details)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Response::Choice, finish_reason, index, message, logprobs)

    // 顶层请求和响应对象
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Request, messages, model, frequency_penalty, max_tokens, presence_penalty, response_format, stop, stream, stream_options, temperature, top_p, tools, tool_choice, logprobs, top_logprobs)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Response, id, choices, created, model, system_fingerprint, object, usage)

    //===========================
    // AutoTest系统结构体
    //===========================

    // 错误样例结构
    struct WAData{
        string in;
        string out;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(WAData,in,out)

    // AutoTest 全局设置结构体
    struct GlobalSetting{
        string openai_url="https://api.openai.com/v1";
        string model="gpt-3.5-turbo";
        string named_model="gpt-4";
        int floder_number=0;
        string allow_path="test_path";
        bool attach_mode=true;

        struct ModelConfig{
            double temperature=0.7;
            int max_tokens=4096;
            double top_p=1.0;
        };
        ModelConfig model_config;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(GlobalSetting::ModelConfig,temperature,max_tokens,top_p)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(GlobalSetting,openai_url,model,named_model,floder_number,allow_path,attach_mode,model_config)

    // Prompt结构体
    struct Prompt{
        string generators;
        string validators;
        string checkers;
        string system;
        string askname;
        std::unordered_map<string,string> custom_prompts;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Prompt,generators,validators,checkers,system,askname,custom_prompts)

    // 测试配置结构体
    struct TestConfig{
        string name;
        bool attach_global=true;
        int now_data=0;
        int now_test=0;
        string data_num;
        int special=2;
        int edge=2;
        int error_limit=2;
        int time_limit=1000;  // ms
        int mem_limit=256;    // MB
        string judge_status="waiting";
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(TestConfig,name,attach_global,now_data,now_test,data_num,special,edge,error_limit,time_limit,mem_limit,judge_status)

    // 对拍统计结构体
    struct TestStats{
        int total_tests=0;
        int passed_tests=0;
        int failed_tests=0;
        int special_tests=0;
        int edge_tests=0;
        struct TestCase{
            int id;
            string status;
            double time_used; // ms
            int memory_used;  // KB
            string error_type;
        };
        std::vector<TestCase> test_cases;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(TestStats::TestCase,id,status,time_used,memory_used,error_type)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(TestStats,total_tests,passed_tests,failed_tests,special_tests,edge_tests,test_cases)

    // AI会话历史记录
    struct SessionHistory{
        std::vector<Request::Message> messages;
        string current_topic;
        bool tool_usage=false;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SessionHistory,messages,current_topic,tool_usage)

} // namespace ns

#endif // AUTO_OPEN_H
