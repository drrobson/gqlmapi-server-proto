#include <iostream>

#include <oatpp/network/Server.hpp>
#include <oatpp/network/tcp/server/ConnectionProvider.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <oatpp/web/server/HttpConnectionHandler.hpp>

#include "controller/GqlQueryController.h"
#include "resolver/MapiResolver.h"

void RunServer()
{
    /* Create Router for HTTP requests routing */
    auto router = oatpp::web::server::HttpRouter::createShared();

    auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
    auto mapiResolver = MapiResolver::Create(false);
    router->addController(std::make_shared<GqlQueryController>(objectMapper, mapiResolver));

    /* Create HTTP connection handler with router */
    auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(router);

    /* Create TCP connection provider */
    auto connectionProvider = oatpp::network::tcp::server::ConnectionProvider::createShared({"localhost", 8000, oatpp::network::Address::IP_4});

    /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
    oatpp::network::Server server(connectionProvider, connectionHandler);

    /* Print info about server port */
    OATPP_LOGI("GqlMapiServer", "Server running on port %s", connectionProvider->getProperty("port").getData());

    /* Run server */
    server.run();
}

int main(int, char **)
{
    /* Init oatpp Environment */
    oatpp::base::Environment::init();

    try
    {
        RunServer();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    

    /* Destroy oatpp Environment */
    oatpp::base::Environment::destroy();
}
