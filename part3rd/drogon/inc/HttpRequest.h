#pragma once

#include "./exports.h"
#include "drogon/utils/Utilities.h"
#include "drogon/HttpTypes.h"
#include "json/json.h"
#include "trantor/net/InetAddress.h"
#include "trantor/net/Certificate.h"
#include "trantor/utils/Date.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <optional>
#include <string_view>
#include "trantor/net/TcpConnection.h"
#include "drogon/DrClassMap.h"
#include "drogon/Session.h"

namespace trantor
{
class HttpRequest;
using HttpRequestPtr = std::shared_ptr<HttpRequest>;


}  // namespace trantor