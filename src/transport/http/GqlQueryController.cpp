#include "GqlQueryController.h"

oatpp::Object<GqlQueryResponse> GqlQueryController::executeQueryRequest(oatpp::String query, oatpp::String variables, oatpp::String operationName)
{
    OATPP_LOGD("[executeQueryRequest]", "query params: query=\"%s\", variables=\"%s\", operation_name=\"%s\"",
               query->c_str(), variables.getValue("<ABSENT>").c_str(), operationName.getValue("<ABSENT>").c_str());

    auto result = m_resolver->Resolve(*(query.get()), variables.getValue(""), operationName.getValue(""));

    if (!result->errors.empty())
    {
        // We encountered a failure upstream from contacting the datastore
        auto responseDto = GqlQueryResponse::createShared();
        responseDto->errors = {};
        std::transform(result->errors.begin(), result->errors.end(), std::back_inserter(*(responseDto->errors)), [](std::string &error)
                       { return oatpp::String{std::move(error)}; });
        return responseDto;
    }
    else if (result->document)
    {
        // We specifically use the JSON serialization helper (instead of our API helper) because
        // the IResolver gives us back a JSON document
        auto jsonSerializationUtils = oatpp::parser::json::mapping::ObjectMapper::createShared();
        return jsonSerializationUtils->readFromString<oatpp::Object<GqlQueryResponse>>(result->document.value());
    }
    else
    {
        OATPP_ASSERT(false);
        return nullptr;
    }
}