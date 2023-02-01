#pragma once

#include <oatpp-websocket/AsyncConnectionHandler.hpp>

struct IWSFrameListenerFactory;

class WSConnectionListener : public oatpp::websocket::AsyncConnectionHandler::SocketInstanceListener
{
public:
    WSConnectionListener(const std::shared_ptr<IWSFrameListenerFactory>& frameListenerFactory);

    virtual void onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket, const std::shared_ptr<const ParameterMap> &params) override;
    virtual void onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket) override;

private:
    static constexpr const char* TAG = "WSConnectionHandler";

    std::shared_ptr<IWSFrameListenerFactory> m_frameListenerFactory;
};