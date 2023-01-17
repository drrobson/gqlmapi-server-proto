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

struct IResolver
{
    virtual std::shared_ptr<ResolveResult> Resolve(std::string query, std::string variables, std::string operationName) = 0;
};