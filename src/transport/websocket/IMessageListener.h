#pragma once

#include <memory>
#include <string>

struct IMessageDispatcher;

struct IMessageListener
{
    virtual void OnChannelClosed(std::string closingMessage) = 0;
    virtual void OnNextMessage(std::string message, const std::shared_ptr<IMessageDispatcher>& responder) = 0;
};

struct IMessageListenerFactory
{
    virtual std::shared_ptr<IMessageListener> Create() = 0;
};