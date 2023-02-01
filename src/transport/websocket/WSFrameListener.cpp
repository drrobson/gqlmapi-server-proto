#include "WSFrameListener.h"

#include "transport/schema/GqlRequest.h"
#include "transport/schema/GqlSubscriptionMessage.h"

#include "transport/websocket/WSMessageDispatcher.h"
#include "transport/websocket/IMessageListener.h"

using namespace oatpp::websocket;

WSFrameListener::WSFrameListener(std::shared_ptr<IMessageListener> messageListener)
    : m_messageListener(messageListener)
    , m_writeLock(std::make_shared<oatpp::async::Lock>())
{
}

oatpp::async::CoroutineStarter WSFrameListener::onPing(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message)
{
    OATPP_LOGD(TAG, "onPing");
    return oatpp::async::synchronize(m_writeLock.get(), socket->sendPongAsync(message));
}

oatpp::async::CoroutineStarter WSFrameListener::onPong(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message)
{
    OATPP_LOGD(TAG, "onPong");
    return nullptr; // do nothing
}

oatpp::async::CoroutineStarter WSFrameListener::onClose(const std::shared_ptr<AsyncWebSocket>& socket, v_uint16 code, const oatpp::String& message)
{
    OATPP_LOGD(TAG, "onClose code=%d", code);
    m_messageListener->OnChannelClosed(message.getValue("" /*default*/));

    // TODO: we should wrap the IMessageListener::OnNextMessage in a coroutine so it can be dispatched without blocking the socket
    return nullptr;
}

oatpp::async::CoroutineStarter WSFrameListener::readMessage(const std::shared_ptr<AsyncWebSocket>& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size)
{
    OATPP_LOGD(TAG, "readMessage opcode=%d size=%d", opcode, size);

    if (size == 0)
    {
        // We now have the complete message in the buffer, extract it and clear the buffer
        oatpp::String completeMessage = m_messageBuffer.toString();
        m_messageBuffer.reset();

        OATPP_LOGD(TAG, "messageReceived size=%d message=\"%s\"", completeMessage->size(), completeMessage->c_str());

        auto messageResponseChannel = std::make_shared<WSMessageDispatcher>(socket, m_writeLock);
        m_messageListener->OnNextMessage(std::move(*(completeMessage.get())), messageResponseChannel);
    }
    else
    {
        if (m_messageBuffer.getCurrentPosition() == 0)
        {
            if (opcode != Frame::OPCODE_TEXT)
                throw std::invalid_argument(std::format("Sending non-text message frames is not yet supported, opcode={}", opcode));
        }
        else
        {
            if (opcode != Frame::OPCODE_CONTINUATION && opcode != Frame::OPCODE_TEXT)
                throw std::invalid_argument(std::format("We have buffered frame data but have received a non-continuation frame, opcode={}", opcode));
        }

        // Received a part of a message; so write it to the buffer
        m_messageBuffer.writeSimple(data, size);
    }

    // TODO: we should wrap the IMessageListener::OnNextMessage in a coroutine so it can be dispatched without blocking the socket
    return nullptr;
}

WSFrameListenerFactory::WSFrameListenerFactory(std::shared_ptr<IMessageListenerFactory> messageListenerFactory)
    : m_messageListenerFactory(messageListenerFactory)
{}

std::shared_ptr<oatpp::websocket::AsyncWebSocket::Listener> WSFrameListenerFactory::Create(
    const std::shared_ptr<const oatpp::network::ConnectionHandler::ParameterMap>& connectionParams)
{
    OATPP_LOGD("WSFrameListenerFactory", "Create called with %d params", connectionParams ? connectionParams->size() : 0);

    return std::make_shared<WSFrameListener>(m_messageListenerFactory->Create());
}