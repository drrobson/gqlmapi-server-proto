#pragma once

#include <oatpp/core/async/Lock.hpp>
#include <oatpp-websocket/AsyncWebSocket.hpp>

#include "AppComponents.h"
#include "IMessageDispatcher.h"

struct WSMessageDispatcher : public IMessageDispatcher
{
    WSMessageDispatcher(const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket, std::shared_ptr<oatpp::async::Lock> lock);

    virtual bool SendMessage(std::string message) override;

private:
    OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutor);

    std::weak_ptr<oatpp::websocket::AsyncWebSocket> m_weakSocket;
    std::shared_ptr<oatpp::async::Lock> m_lock;
};