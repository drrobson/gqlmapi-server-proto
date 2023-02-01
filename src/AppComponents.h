#pragma once

#include <oatpp/core/async/Executor.hpp>
#include <oatpp/core/macro/component.hpp>
#include <oatpp/network/tcp/server/ConnectionProvider.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <oatpp/web/server/HttpConnectionHandler.hpp>

/**
 *  Class which creates and holds Application components and registers components in oatpp::base::Environment
 *  Order of components initialization is from top to bottom
 */
class AppComponent
{
public:
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor)([]
    {
        return std::make_shared<oatpp::async::Executor>(
            4 /* Data-Processing threads */,
            2 /* I/O threads */,
            2 /* Timer threads */);
    }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider)([]
    {
        return oatpp::network::tcp::server::ConnectionProvider::createShared({"localhost", 8000, oatpp::network::Address::IP_4});
    }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)([]
    {
        return oatpp::web::server::HttpRouter::createShared();
    }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, httpConnectionHandler)("httpConnectionHandler", []
    {
        OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter); // get router
        return oatpp::web::server::HttpConnectionHandler::createShared(httpRouter);
    }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)([]
    {
        return oatpp::parser::json::mapping::ObjectMapper::createShared();
    }());
};