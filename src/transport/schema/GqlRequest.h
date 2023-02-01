#pragma once

#include <oatpp/core/macro/codegen.hpp>

/* Begin DTO code-generation */
#include OATPP_CODEGEN_BEGIN(DTO)

class GqlQueryRequest : public oatpp::DTO
{
    DTO_INIT(GqlQueryRequest, DTO /* Extends */)

    DTO_FIELD(String, query);
    DTO_FIELD(oatpp::Fields<oatpp::Any>, variables);
    DTO_FIELD(String, operationName);
};

/* End DTO code-generation */
#include OATPP_CODEGEN_END(DTO)