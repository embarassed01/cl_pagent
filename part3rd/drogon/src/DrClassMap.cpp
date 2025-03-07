#include "drogon/DrClassMap.h"
#include "drogon/DrObject.h"
#include "trantor/utils/Logger.h"

using namespace drogon;

namespace drogon
{
namespace internal
{
static std::unordered_map<std::string, std::shared_ptr<DrObjectBase>> & 
getObjsMap()
{
    static std::unordered_map<std::string, std::shared_ptr<DrObjectBase>> 
        singleInstanceMap;
    return singleInstanceMap;
}

static std::mutex &getMapMutex()
{
    static std::mutex mtx;
    return mtx;
}
}  // namespace internal
}  // namespace dorgon

void DrClassMap::registerClass(const std::string &className, 
                                const DrAllocFunc &func,
                                const DrSharedAllocFunc &sharedFunc)
{
    LOG_TRACE << "Register class:" << className;
    getMap().insert(
        std::make_pair(className, std::make_pair(func, sharedFunc)));
}

DrObjectBase *DrClassMap::newObject(const std::string &className)
{
    auto iter = getMap().find(className);
    if (iter != getMap().end())
    {
        return iter->second.first();
    }
    else 
        return nullptr;
}

std::shared_ptr<DrObjectBase> DrClassMap::newSharedObject(
        const std::string &className)
{
    auto iter = getMap().find(className);
    if (iter != getMap().end())
    {
        if (iter->second.second)
            return iter->second.second();
        else 
            return std::shared_ptr<DrObjectBase>(iter->second.first());
    }
    else 
        return nullptr;
}

// singleton
std::unordered_map<std::string, std::pair<DrAllocFunc, DrSharedAllocFunc>> &
DrClassMap::getMap()
{
    static std::unordered_map<std::string,
                                std::pair<DrAllocFunc, DrSharedAllocFunc>>
        map;
    return map;
}