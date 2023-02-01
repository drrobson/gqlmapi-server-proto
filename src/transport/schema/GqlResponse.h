#pragma once

#include <oatpp/core/macro/codegen.hpp>

/* Begin DTO code-generation */
#include OATPP_CODEGEN_BEGIN(DTO)

class GqlQueryResponse : public oatpp::DTO
{
    DTO_INIT(GqlQueryResponse, DTO /* Extends */)

    DTO_FIELD(oatpp::Fields<oatpp::Any>, data);
    DTO_FIELD(oatpp::Vector<oatpp::Any>, errors);
};

/* End DTO code-generation */
#include OATPP_CODEGEN_END(DTO)