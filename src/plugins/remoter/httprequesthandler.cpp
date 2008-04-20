#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "httprequesthandler.h"

void HTTPRequestHandler::handleRequest (Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
    response.setContentType ("text/html");
    std::ostream& ostr = response.send ();
    ostr << "<html><head><title>Test</title></head><body>Test (url " << request.getURI () << ") </body></html>";
}

