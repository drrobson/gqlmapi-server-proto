#pragma once

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <oatpp/web/server/api/ApiController.hpp>

#include <oatpp-websocket/Handshaker.hpp>

#include "AppComponents.h"
#include "graphql/public/IResolver.h"
#include "transport/schema/GqlRequest.h"
#include "transport/schema/GqlResponse.h"
#include "transport/subscription/SubscriptionMessageListener.h"
#include "transport/websocket/WSConnectionListener.h"
#include "transport/websocket/WSFrameListener.h"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class GqlQueryController : public oatpp::web::server::api::ApiController
{
public:
    GqlQueryController(std::shared_ptr<oatpp::data::mapping::ObjectMapper> apiSerializationUtils, std::shared_ptr<IResolver> resolver)
        : ApiController(apiSerializationUtils)
        , m_apiSerializationUtils(apiSerializationUtils)
        , m_resolver(resolver)
    {
        // Configure the handler for the graphql/subscription endpoint
        auto frameListenerFactory = std::make_shared<WSFrameListenerFactory>(std::make_shared<SubscriptionMessageListenerFactory>(resolver));
        auto connectionListener = std::make_shared<WSConnectionListener>(frameListenerFactory);
        m_wsSubscriptionHandler = oatpp::websocket::AsyncConnectionHandler::createShared();
        m_wsSubscriptionHandler->setSocketInstanceListener(connectionListener);
    }

    ADD_CORS(executeQueryViaGet, "*", "GET");
    ENDPOINT("GET", "/graphql", executeQueryViaGet,
             REQUEST(std::shared_ptr<IncomingRequest>, request)
             )
    {
        // TODO: Decode these URL parameters - oatpp does not do this automatically
        // when version 1.4.0 is availalable, that has a built-in url decoder

        auto query = request->getQueryParameter("query");
        auto variables = request->getQueryParameter("variables");
        auto operationName = request->getQueryParameter("operationName");

        auto responseDto = executeQueryRequest(std::move(query), std::move(variables), std::move(operationName));

        return createDtoResponse(Status::CODE_200, responseDto);
    }

    ADD_CORS(executeQueryViaPost, "*", "POST");
    ENDPOINT("POST", "/graphql", executeQueryViaPost,
             BODY_DTO(Object<GqlQueryRequest>, gqlRequest)
             )
    {
        oatpp::String variables = m_apiSerializationUtils->writeToString(gqlRequest->variables);

        auto responseDto = executeQueryRequest(gqlRequest->query, variables, gqlRequest->operationName);

        return createDtoResponse(Status::CODE_200, responseDto);
    }

    ADD_CORS(loadGraphiql, "*", "GET");
    ENDPOINT("GET", "/graphiql", loadGraphiql)
    {
        return createResponse(Status::CODE_200, oatpp::String::loadFromFile("graphiql.html"));
    }

    ADD_CORS(graphqlSubscription, "*", "GET");
    ENDPOINT("GET", "/graphql/subscription", graphqlSubscription,
             REQUEST(std::shared_ptr<IncomingRequest>, request)
            )
    {
        return oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), m_wsSubscriptionHandler);
    }

private:
    oatpp::Object<GqlQueryResponse> executeQueryRequest(oatpp::String query, oatpp::String variables, oatpp::String operationName);

    std::shared_ptr<oatpp::data::mapping::ObjectMapper> m_apiSerializationUtils;

    std::shared_ptr<IResolver>  m_resolver;
    std::shared_ptr<oatpp::websocket::AsyncConnectionHandler> m_wsSubscriptionHandler;
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen