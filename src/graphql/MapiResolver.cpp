#include "graphql/MapiResolver.h"

#include <graphqlservice/JSONResponse.h>

using namespace graphql;

struct GqlMapiQueryParams
{
    GqlMapiQueryParams(peg::ast&& parsedQuery, response::Value&& parsedVariables, std::string&& operationName)
        : parsedQuery(std::move(parsedQuery))
        , parsedVariables(std::move(parsedVariables))
        , operationName(std::move(operationName))
    {}



    peg::ast parsedQuery;
    response::Value parsedVariables;
    std::string operationName;
};

std::shared_ptr<GqlMapiQueryParams> PrepareQueryParams(std::string query, std::string variables, std::string operationName)
{
    peg::ast parsedQuery;
    try
    {
        parsedQuery = peg::parseString(query);
    }
    catch (std::exception e)
    {
        throw std::runtime_error(std::format("Failed to parse query: {}", e.what()));
    }

    response::Value parsedVariables = variables.empty() ? response::Value(response::Type::Map) : response::parseJSON(variables);
    if (parsedVariables.type() != response::Type::Map)
    {
        // we may have received a string value of "null" -- Graphiql does this -- so handle that case
        if (variables == "null")
        {
            parsedVariables = response::Value(response::Type::Map);
        }
        else
        {
            throw std::runtime_error(std::format("Failed to parse supplied variables: {}", variables));
        }
    }

    return std::make_shared<GqlMapiQueryParams>(std::move(parsedQuery), std::move(parsedVariables), std::move(operationName));
}

std::shared_ptr<ResolveResult> MapiResolver::Resolve(std::string query, std::string variables, std::string operationName)
{
    auto result = std::make_shared<ResolveResult>();
    std::shared_ptr<GqlMapiQueryParams> queryParams;
    try
    {
        queryParams = PrepareQueryParams(query, variables, operationName);
    }
    catch (std::exception e)
    {
        result->errors.push_back(e.what());
        return result;
    }
    
    // const auto asyncStrategy = std::make_shared<service::await_worker_thread>();

    response::Value response;
    try
    {
        service::RequestResolveParams params { queryParams->parsedQuery, queryParams->operationName, response::Value(queryParams->parsedVariables) };
        // TODO: either turn this entire Resolve function into a coroutine so we can co_await this OR accept a callback in the
        // function signature and chain a callback off an await on the result
        auto awaitableResponse = m_gqlmapiService->resolve(params);
        response = awaitableResponse.get();
    }
    catch (std::exception e)
    {
        result->errors.push_back(e.what());
        return result;
    }

    result->document = response::toJSON(std::move(response));

    return result;
}

std::shared_ptr<SubscriptionKey> MapiResolver::Subscribe(
      std::string query
    , std::string variables
    , std::string operationName
    , std::shared_ptr<ISubscriptionCallbacks> callbacks)
{
    try
    {
        auto queryParams = PrepareQueryParams(query, variables, operationName);
        if (m_gqlmapiService->findOperationDefinition(queryParams->parsedQuery, queryParams->operationName).first != service::strSubscription)
            throw std::runtime_error("Attempting to use subscribe semantics for a non-subscription GraphQL operation");

        // TODO: the `launch` param given here is used by service::Request::deliver, in GraphQLService.cpp, to decide
        // what thread to invoke the subscription callback from -- by default it is synchronous, i.e. from the MAPI notification
        // thread. Enforcing sequential delivery up to the server layer doesn't seem like a bad thing - we can parallelize there if desired
        // but if we want to allow parallel dispatching of subscription notifications, then pass a launch param to that effect here
        // the other logic behind the launch gate is the actual execution of the GraphQL subscription document on the source events
        // so it may be safer to shell that to a separate thread: what happens if the document execution causes an exception - does that
        // crash the app? crash the notification pump? Needs investigation
        auto subscriptionKey = m_gqlmapiService->subscribe(
            service::RequestSubscribeParams {
                [callbacks](response::Value response)
                {
                    auto result = std::make_shared<ResolveResult>();
                    result->document = response::toJSON(std::move(response));
                    callbacks->OnNextResult(result);
                },
                std::move(queryParams->parsedQuery),
                std::move(queryParams->operationName),
                std::move(queryParams->parsedVariables)
            }).get();
        
        SubscriptionKey key = std::to_string(subscriptionKey);
        callbacks->OnRegistered(key);

        return std::make_shared<SubscriptionKey>(std::move(key));
    }
    catch (std::exception e)
    {
        callbacks->OnError(std::format("Error encountered initiating the subscription: {}", e.what()));

        return nullptr;
    }
}

void MapiResolver::Unsubscribe(const SubscriptionKey& key)
{
    service::SubscriptionKey subscriptionKey {};
    auto [ptr, ec] { std::from_chars(key.data(), key.data() + key.size(), subscriptionKey) };

    if (ec == std::errc())
    {
        m_gqlmapiService->unsubscribe(service::RequestUnsubscribeParams { subscriptionKey }).get();
    }
    else
    {
        throw std::runtime_error(std::format("Failed to convert the subscription key into necessary key type, key=\"{}\"", key));
    }
}