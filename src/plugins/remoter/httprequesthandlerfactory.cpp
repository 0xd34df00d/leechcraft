#include <Poco/Net/HTTPServerRequest.h>
#include "httprequesthandler.h"
#include "httprequesthandlerfactory.h"

Poco::Net::HTTPRequestHandler* HTTPRequestHandlerFactory::createRequestHandler (const Poco::Net::HTTPServerRequest& request)
{
    return new HTTPRequestHandler ();
}

