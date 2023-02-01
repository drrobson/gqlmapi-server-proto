#include "WSMessageDispatcher.h"

#include <oatpp/core/async/Coroutine.hpp>

WSMessageDispatcher::WSMessageDispatcher(const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket, std::shared_ptr<oatpp::async::Lock> lock)
    : m_weakSocket(socket)
    , m_lock(std::move(lock))
{
}

class SendMessageCoroutine : public oatpp::async::Coroutine<SendMessageCoroutine>
{
public:
    SendMessageCoroutine(std::shared_ptr<oatpp::async::Lock> lock, std::shared_ptr<oatpp::websocket::AsyncWebSocket> socket, std::string message)
        : m_lock(std::move(lock))
        , m_socket(std::move(socket))
        , m_message(std::move(message))
    {}

    virtual Action act() override
    {
        return oatpp::async::synchronize(m_lock.get(), m_socket->sendOneFrameTextAsync(std::move(m_message)))
                .next(finish());
    }

private:
    std::shared_ptr<oatpp::async::Lock> m_lock;
    std::shared_ptr<oatpp::websocket::AsyncWebSocket> m_socket;
    std::string m_message;
};

bool WSMessageDispatcher::SendMessage(std::string message)
{
    auto socket = m_weakSocket.lock();
    if (!socket)
        return false;

    m_asyncExecutor->execute<SendMessageCoroutine>(m_lock, socket, std::move(message));
    return true;
}