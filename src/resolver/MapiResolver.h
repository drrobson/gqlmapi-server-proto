#pragma once

#include <MAPIGraphQL.h>

#include "IResolver.h"

class MapiResolver : public IResolver
{
public:
    static std::shared_ptr<MapiResolver> Create(bool useDefaultProfile);
    
    MapiResolver(std::shared_ptr<graphql::service::Request> gqlmapiService)
        : m_gqlmapiService(gqlmapiService)
    {}

    virtual std::shared_ptr<ResolveResult> Resolve(std::string query, std::string variables, std::string operationName) override;

private:

    std::shared_ptr<graphql::service::Request> m_gqlmapiService;
};