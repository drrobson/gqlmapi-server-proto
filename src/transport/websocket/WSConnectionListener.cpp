#include "WSConnectionListener.h"

#include "WSFrameListener.h"

WSConnectionListener::WSConnectionListener(const std::shared_ptr<IWSFrameListenerFactory>& frameListenerFactory)
    : m_frameListenerFactory(frameListenerFactory)
{}

void WSConnectionListener::onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket, const std::shared_ptr<const ParameterMap>& params)
{
    OATPP_LOGD(TAG, "websocket connection created");

    socket->setListener(m_frameListenerFactory->Create(params));
}

void WSConnectionListener::onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket)
{
    OATPP_LOGD(TAG, "websocket connection destroyed");
}