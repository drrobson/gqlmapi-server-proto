#pragma once

#include <functional>
#include <memory>

#include "AppComponents.h"

#include "graphql/public/IResolver.h"
#include "transport/websocket/IMessageListener.h"

struct ISubscriptionMessageDispatcher;

class SubscriptionCallbackSink : public ISubscriptionCallbacks
{
public:
    SubscriptionCallbackSink( std::shared_ptr<IMessageDispatcher> messageChannel
                            , std::function<void(const SubscriptionId&)> unsubscribeFunc);

    virtual void OnRegistered(const SubscriptionId& id) override;
    virtual void OnError(std::string errorMessage) override;
    virtual void OnNextResult(const std::shared_ptr<const ResolveResult>& result) override;

private:
    void SendSubscriptionMessage(std::string messageName, std::optional<std::string> payload);

    OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, m_apiObjectMapper);

    std::shared_ptr<IMessageDispatcher> m_messageChannel;
    std::optional<SubscriptionId> m_subscriptionId;
    std::function<void(const SubscriptionId&)> m_unsubscribeFunc;
};

struct SubscriptionMessageListener : public IMessageListener
{
    SubscriptionMessageListener(std::shared_ptr<IResolver> resolver);

    virtual void OnChannelClosed(std::string closingMessage) override;
    virtual void OnNextMessage(std::string message, const std::shared_ptr<IMessageDispatcher>& responder) override;

private:
    static constexpr const char* TAG = "SubscriptionMessageListener";

    OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, m_apiObjectMapper);
    OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutor);
    
    std::shared_ptr<IResolver> m_resolver;
    std::unordered_set<SubscriptionId> m_activeSubscriptionIds;
    std::mutex m_activeSubscriptionIdsLock;
};

struct SubscriptionMessageListenerFactory : public IMessageListenerFactory
{
    SubscriptionMessageListenerFactory(std::shared_ptr<IResolver> resolver);

    virtual std::shared_ptr<IMessageListener> Create() override;

private:
    std::shared_ptr<IResolver> m_resolver;
};