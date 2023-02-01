#pragma once

#include <MAPIGraphQL.h>

#include "graphql/public/IResolver.h"

class MapiResolver : public IResolver
{
public:
    MapiResolver(std::shared_ptr<graphql::service::Request> gqlmapiService)
        : m_gqlmapiService(gqlmapiService)
    {}

    virtual std::shared_ptr<ResolveResult>  Resolve(
                              std::string query
                            , std::string variables
                            , std::string operationName) override;

    virtual std::shared_ptr<SubscriptionId> Subscribe(
                              std::string query
                            , std::string variables
                            , std::string operationName
                            , std::shared_ptr<ISubscriptionCallbacks> callbacks) override;

    virtual void                            Unsubscribe(const SubscriptionId& id) override;

private:
    std::shared_ptr<graphql::service::Request> m_gqlmapiService;
};