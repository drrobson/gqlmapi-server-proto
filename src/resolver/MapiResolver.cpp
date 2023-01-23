#include <graphqlservice/JSONResponse.h>

#include "MapiResolver.h"

/*static*/ std::shared_ptr<MapiResolver> MapiResolver::Create(bool useDefaultProfile)
{
    auto service = graphql::mapi::GetService(useDefaultProfile);

    return std::make_shared<MapiResolver>(service);
}

std::shared_ptr<ResolveResult> MapiResolver::Resolve(std::string query, std::string variables, std::string operationName)
{
    using namespace graphql;

    auto result = std::make_shared<ResolveResult>();
    
    peg::ast parsedQuery;
    try
    {
        parsedQuery = peg::parseString(query);
    }
    catch (std::exception e)
    {
        result->errors.push_back(std::format("Failed to parse query: {}", e.what()));
        return result;
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
            result->errors.push_back(std::format("Failed to parse supplied variables: {}", variables));
            return result;
        }
    }

    // const auto asyncStrategy = std::make_shared<service::await_worker_thread>();

    response::Value response;
    try
    {
        // TODO: either turn this entire Resolve function into a coroutine so we can co_await this OR accept a callback in the
        // function signature and chain a callback off an await on the result
        auto awaitableResponse = m_gqlmapiService->resolve(
            service::RequestResolveParams{parsedQuery, operationName, std::move(parsedVariables)/*, service::await_async(asyncStrategy)*/});
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