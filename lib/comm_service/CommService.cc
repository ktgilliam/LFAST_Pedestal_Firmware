
#include "CommService.h"

#include <array>
#include <sstream>
#include <iterator>
#include <cstring>
#include <string>
#include <cstdlib>
// #include <StreamUtils.h>
#include <algorithm>
// #include <string_view>

#include <debug.h>
#include <device.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
LFAST::CommsService::CommsService()
{
    // for (uint16_t ii = 0; ii < MAX_CTRL_MESSAGES; ii++)
    // {
    //     this->registerMessageHandler(ii, CommsService::defaultMessageHandler);
    // }
    // this->messageHandlerList["Default"] = CommsService::defaultMessageHandler;
}

// void LFAST::CommsService::registerMessageHandler(std::string key, MsgHandlerFn fn)
// {
//     this->messageHandlerList[key] = fn;
// }

void LFAST::CommsService::defaultMessageHandler(std::string info)
{

    std::stringstream ss;
    ss << "Unregistered Message." << std::endl;
    TEST_SERIAL.println(ss.str().c_str());
}

void LFAST::CommsService::errorMessageHandler(CommsMessage &msg)
{
    std::stringstream ss;
    ss << "Invalid Message.";
    //
    ss << std::endl;
    TEST_SERIAL.println(ss.str().c_str());
}

void LFAST::CommsMessage::parseReceivedData(char *rxBuff)
{
    // rxBuff->erase(std::remove_if(rxBuff->begin(), rxBuff->end(),
    //                              [](char c)
    //                              {
    //                                  return (c == '\r' || c == '\t' || c == ' ' || c == '\n');
    //                              }),
    //               rxBuff->end());
    auto error = deserializeJson(jsonDoc, rxBuff);
    // Test if parsing succeeds.
    if (error)
    {
        TEST_SERIAL.print(F("deserializeJson() failed: "));
        TEST_SERIAL.println(error.f_str());
        return;
    }
}

bool LFAST::CommsService::callMessageHandler(JsonPair kvp)
{
    bool handlerFound = true;
    auto keyStr = std::string(kvp.key().c_str());
    if (this->handlerTypes.find(keyStr) == this->handlerTypes.end())
    {
        handlerFound = false;
        defaultMessageHandler(keyStr);
    }
    else
    {
        auto handlerType = this->handlerTypes[keyStr];

        switch (handlerType)
        {
        case INT_HANDLER:
        {
            auto val = kvp.value().as<int>();
            this->callMessageHandler<int>(keyStr, val);
        }
        break;
        case UINT_HANDLER:
        {
            auto val = kvp.value().as<unsigned int>();
            this->callMessageHandler<unsigned int>(keyStr, val);
        }
        break;
        case DOUBLE_HANDLER:
        {
            auto val = kvp.value().as<double>();
            this->callMessageHandler<double>(keyStr, val);
        }
        break;
        case BOOL_HANDLER:
        {
            auto val = kvp.value().as<bool>();
            this->callMessageHandler<bool>(keyStr, val);
        }
        break;
        case STRING_HANDLER:
        {
            auto val = kvp.value().as<std::string>();
            this->callMessageHandler<std::string>(keyStr, val);
        }
        break;
        default:
            handlerFound = false;
        }
    }

    return handlerFound;
}
// void CommsMessage::printMessageInfo()
// {
//     TEST_SERIAL.printf("%d:\t", id);
//     for (uint16_t ii = 0; ii < args.size(); ii++)
//     {
//         TEST_SERIAL.printf("%4.2f:\t", args.at(ii));
//     }
// }

// std::string CommsMessage::getMessageStr()
// {
//     std::stringstream ss;
//     for (uint16_t ii = 0; ii < args.size(); ii++)
//     {
//         ss << this->args.at(ii) << ";";
//     }
//     ss << std::hex << this->id << std::endl;
//     return ss.str();
// }

void LFAST::CommsService::processReceived()
{

    for (auto &msg : this->messageQueue)
    {
        auto msgRoot = msg->jsonDoc.to<JsonObject>();
        for (JsonPair kvp : msgRoot)
            this->callMessageHandler(kvp);
    }
}