#pragma once

#include <oatpp/core/async/Lock.hpp>
#include <oatpp-websocket/AsyncWebSocket.hpp>
#include <oatpp-websocket/AsyncConnectionHandler.hpp>

struct IMessageListener;

class WSFrameListener : public oatpp::websocket::AsyncWebSocket::Listener
{
public:
    WSFrameListener(std::shared_ptr<IMessageListener> messageListener);

    CoroutineStarter onPing(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) override;
    CoroutineStarter onPong(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) override;
    CoroutineStarter onClose(const std::shared_ptr<AsyncWebSocket>& socket, v_uint16 code, const oatpp::String& message) override;
    CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket>& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;

private:
    static constexpr const char* TAG = "WSFrameListener";

    std::shared_ptr<IMessageListener> m_messageListener;

    oatpp::data::stream::BufferOutputStream m_messageBuffer;
    std::shared_ptr<AsyncWebSocket> m_socket;
    std::shared_ptr<oatpp::async::Lock> m_writeLock;
};

struct IWSFrameListenerFactory
{
    virtual std::shared_ptr<oatpp::websocket::AsyncWebSocket::Listener>
        Create(const std::shared_ptr<const oatpp::websocket::AsyncConnectionHandler::ParameterMap>& connectionParams) = 0;
};

struct IMessageListenerFactory;

class WSFrameListenerFactory : public IWSFrameListenerFactory
{ 
public:
    WSFrameListenerFactory(std::shared_ptr<IMessageListenerFactory> messageListenerFactory);

    virtual std::shared_ptr<oatpp::websocket::AsyncWebSocket::Listener>
        Create(const std::shared_ptr<const oatpp::websocket::AsyncConnectionHandler::ParameterMap>& connectionParams) override;

private:
    std::shared_ptr<IMessageListenerFactory> m_messageListenerFactory;
};