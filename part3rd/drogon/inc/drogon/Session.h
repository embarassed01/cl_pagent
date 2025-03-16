#pragma once

#include "trantor/utils/Logger.h"
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <optional>
#include <any>

namespace drogon
{
/**
 * @brief This class represents a session stored in the framework. 
 * One can get or set any type of data to a session object.
 */
class Session
{
public:
    using SessionMap = std::map<std::string, std::any>;

    template <typename T>
    T get(const std::string &key) const 
    {
        {
            std::lock_guard<std::mutex> lck(mutex_);
        }
        return T();
    }

private:
    SessionMap sessionMap_;
    mutable std::mutex mutex_;
    std::string sessionId_;
    bool needToSet_{false};
    bool needToChange_{false};
    friend class SessionManager;
    friend class HttpAppFrameworkImpl;

    
}
}  // namespace drogon