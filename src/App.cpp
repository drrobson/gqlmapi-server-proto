#include <iostream>

#include <oatpp/network/Server.hpp>

#include "AppComponents.h"

#include "transport/http/GqlQueryController.h"
#include "graphql/MapiResolver.h"

void RunServer()
{
    // Create + register our oatpp app components
    AppComponent components;

    OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiSerializationUtils);

    auto gqlmapiService = graphql::mapi::GetService(false /*useDefaultProfile*/);
    auto mapiResolver = std::make_shared<MapiResolver>(std::move(gqlmapiService));
    auto httpController = std::make_shared<GqlQueryController>(apiSerializationUtils, mapiResolver);

    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter);
    httpRouter->addController(httpController);

    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, httpConnectionHandler, "httpConnectionHandler");
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

    oatpp::network::Server server(connectionProvider, httpConnectionHandler);
    OATPP_LOGI("GqlMapiServer", "Server launching on port %s", connectionProvider->getProperty("port").getData());

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
