#pragma once

#include <optional>
#include <memory>
#include <string>
#include <vector>

struct ResolveResult
{
    std::optional<std::string> document; // contains top-level data and errors fields
    std::vector<std::string> errors;
};

using SubscriptionId = std::string;

struct ISubscriptionCallbacks
{
    virtual void OnRegistered(const SubscriptionId& id) = 0;
    virtual void OnError(std::string errorMessage) = 0;
    virtual void OnNextResult(const std::shared_ptr<const ResolveResult>& result) = 0;
};

struct IResolver
{
    virtual std::shared_ptr<ResolveResult>  Resolve(
                              std::string query
                            , std::string variables
                            , std::string operationName) = 0;

    virtual std::shared_ptr<SubscriptionId> Subscribe(
                              std::string query
                            , std::string variables
                            , std::string operationName
                            , std::shared_ptr<ISubscriptionCallbacks> callbacks) = 0;

    virtual void                            Unsubscribe(const SubscriptionId& id) = 0;
};