#include "SubscriptionMessageListener.h"

#include <oatpp/core/async/Coroutine.hpp>
#include <oatpp/core/Types.hpp>

#include "graphql/public/IResolver.h"
#include "transport/schema/GqlRequest.h"
#include "transport/schema/GqlSubscriptionMessage.h"
#include "transport/websocket/IMessageDispatcher.h"


SubscriptionCallbackSink::SubscriptionCallbackSink(   SubscriptionId subscriptionId
                                                    , std::shared_ptr<IMessageDispatcher> messageChannel
                                                    , std::function<void(const SubscriptionId& id)> unsubscribeFunc)
    : m_subscriptionId(std::move(subscriptionId))
    , m_messageChannel(std::move(messageChannel))
    , m_unsubscribeFunc(std::move(unsubscribeFunc))
{}

void SubscriptionCallbackSink::OnRegistered(const SubscriptionKey& key)
{
    m_subscriptionKey = key;
    SendSubscriptionMessage("subscription_established", std::nullopt);
}

void SubscriptionCallbackSink::OnError(std::string errorMessage)
{
    SendSubscriptionMessage("subscription_error", errorMessage);
}

void SubscriptionCallbackSink::OnNextResult(const std::shared_ptr<const ResolveResult>& result)
{
    // TODO: do I need to check the error portion of the result to decide whether to write that?
    SendSubscriptionMessage("subscription_event", result->document);
}

void SubscriptionCallbackSink::SendSubscriptionMessage(std::string messageName, std::optional<std::string> payload)
{
    auto message = GqlSubscriptionMessage::createShared();
    message->messageName = std::move(messageName);
    message->subscriptionId = m_subscriptionId;

    if (payload)
        message->payload = std::move(payload.value());

    if (!m_messageChannel->SendMessage(m_apiObjectMapper->writeToString(message)))
    {
        // If we no longer have a message channel, we should unsubscribe because nobody is listening
        if (m_subscriptionKey)
            m_unsubscribeFunc(*m_subscriptionKey);
    }
}

SubscriptionMessageListener::SubscriptionMessageListener(std::shared_ptr<IResolver> resolver)
    : m_resolver(std::move(resolver))
{
}

class BatchUnsubscribeCoroutine : public oatpp::async::Coroutine<BatchUnsubscribeCoroutine>
{
public:
    BatchUnsubscribeCoroutine(std::unordered_map<SubscriptionId, SubscriptionKey> subscriptions, std::shared_ptr<IResolver> resolver)
        : m_subscriptions(std::move(subscriptions))
        , m_resolver(std::move(resolver))
    {}

    virtual Action act() override
    {
        for (const auto& subscription : m_subscriptions)
        {
            m_resolver->Unsubscribe(subscription.second);
        }

        return finish();
    }

private:
    std::unordered_map<SubscriptionId, SubscriptionKey> m_subscriptions;
    std::shared_ptr<IResolver> m_resolver;
};

void SubscriptionMessageListener::OnChannelClosed(std::string closingMessage)
{
    std::unordered_map<SubscriptionId, SubscriptionKey> activeSubscriptions;
    {
        std::lock_guard<std::mutex> lock(m_activeSubscriptionsLock);
        activeSubscriptions = std::move(m_activeSubscriptions);
    }

    m_asyncExecutor->execute<BatchUnsubscribeCoroutine>(std::move(activeSubscriptions), m_resolver);
}

void SubscriptionMessageListener::OnNextMessage(std::string message, const std::shared_ptr<IMessageDispatcher>& responder)
{
    try
    {
        auto subscriptionMessage = m_apiObjectMapper->readFromString<oatpp::Object<GqlSubscriptionMessage>>(message);
        if (subscriptionMessage->subscriptionId.get() == nullptr)
            throw std::invalid_argument("unsubscribe message did not specify \"subscriptionId\", i.e. which subscription to unsubscribe from");
        
        if (subscriptionMessage->messageName == "subscribe")
        {
            auto gqlRequest = m_apiObjectMapper->readFromString<oatpp::Object<GqlQueryRequest>>(subscriptionMessage->payload);
            OATPP_LOGI(TAG, "subscribe request received with id=\"%s\", query=\"%s\"", subscriptionMessage->subscriptionId->c_str(), gqlRequest->query->c_str());

            auto variables = m_apiObjectMapper->writeToString(gqlRequest->variables);

            auto callbackSink = std::make_shared<SubscriptionCallbackSink>(
                  subscriptionMessage->subscriptionId->c_str()
                , responder
                , [weakResolver = std::weak_ptr(m_resolver)](const SubscriptionKey& id)
            {
                auto resolver = weakResolver.lock();
                if (resolver)
                    resolver->Unsubscribe(id);
            });

            auto subscriptionKey = m_resolver->Subscribe(  std::move(*(gqlRequest->query.get()))
                                                        , variables.get() == nullptr ? std::string("") : std::move(*(variables.get()))
                                                        , gqlRequest->operationName.get() == nullptr ? std::string ("") : std::move(*(gqlRequest->operationName.get()))
                                                        , std::move(callbackSink));

            if (subscriptionKey)
            {
                std::lock_guard<std::mutex> lock(m_activeSubscriptionsLock);
                m_activeSubscriptions.emplace(std::move(*(subscriptionMessage->subscriptionId.get())), std::move(*subscriptionKey));
            }
        }
        else if (subscriptionMessage->messageName == "unsubscribe")
        {
            OATPP_LOGI(TAG, "unsubscribe request received with subscriptionId=\"%s\"", subscriptionMessage->subscriptionId->c_str());

            m_resolver->Unsubscribe((*(subscriptionMessage->subscriptionId.get())));

            {
                std::lock_guard<std::mutex> lock(m_activeSubscriptionsLock);
                m_activeSubscriptions.erase(*(subscriptionMessage->subscriptionId.get()));
            }
        }
        else
        {
            throw std::invalid_argument(std::format("Unrecognized message name \"{}\"", subscriptionMessage->messageName->c_str()));
        }
    }
    catch (std::exception e)
    {
        throw std::runtime_error(std::format("Message did not conform to required schema, error=\"{}\"", e.what()));
    }
}

SubscriptionMessageListenerFactory::SubscriptionMessageListenerFactory(std::shared_ptr<IResolver> resolver)
    : m_resolver(std::move(resolver))
{
}

std::shared_ptr<IMessageListener> SubscriptionMessageListenerFactory::Create()
{
    return std::make_shared<SubscriptionMessageListener>(m_resolver);
}