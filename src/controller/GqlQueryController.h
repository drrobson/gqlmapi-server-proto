#pragma once

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <oatpp/web/server/api/ApiController.hpp>

#include "../dto/GqlRequest.h"
#include "../dto/GqlResponse.h"
#include "../resolver/IResolver.h"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class GqlQueryController : public oatpp::web::server::api::ApiController
{
public:
    GqlQueryController(std::shared_ptr<oatpp::data::mapping::ObjectMapper> objectMapper, std::shared_ptr<IResolver> resolver)
        : oatpp::web::server::api::ApiController(objectMapper)
        , m_resolver(resolver)
    {}

    ADD_CORS(executeQueryViaGet, "*", "GET");
    ENDPOINT("GET", "/graphql", executeQueryViaGet,
             REQUEST(std::shared_ptr<IncomingRequest>, request)
             )
    {
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
        auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
        oatpp::String variables = jsonObjectMapper->writeToString(gqlRequest->variables);

        auto responseDto = executeQueryRequest(gqlRequest->query, variables, gqlRequest->operationName);

        return createDtoResponse(Status::CODE_200, responseDto);
    }

    ADD_CORS(loadGraphiql, "*", "GET");
    ENDPOINT("GET", "/graphiql", loadGraphiql)
    {
        return createResponse(Status::CODE_200, oatpp::String::loadFromFile("index.html"));
    }

private:
    oatpp::Object<GqlQueryResponse> executeQueryRequest(oatpp::String query, oatpp::String variables, oatpp::String operationName);

    std::shared_ptr<IResolver>  m_resolver;
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen