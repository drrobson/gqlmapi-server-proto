#pragma once

#include <oatpp/core/macro/codegen.hpp>

/* Begin DTO code-generation */
#include OATPP_CODEGEN_BEGIN(DTO)

class GqlSubscriptionMessage : public oatpp::DTO
{
    DTO_INIT(GqlSubscriptionMessage, DTO /* Extends */)

    DTO_FIELD(oatpp::String, messageName);
    DTO_FIELD(oatpp::String, subscriptionId);
    DTO_FIELD(oatpp::String, payload);
};

/* End DTO code-generation */
#include OATPP_CODEGEN_END(DTO)