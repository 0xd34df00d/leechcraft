#ifndef HTTPREQUESTHANDLER_H
#define HTTPREQUESTHANDLER_H
#include <Poco/Net/HTTPRequestHandler.h>

namespace Poco
{
    namespace Net
    {
        class HTTPServerRequest;
        class HTTPServerResponse;
    };
};

class HTTPRequestHandler : public Poco::Net::HTTPRequestHandler
{
public:
    virtual void handleRequest (Poco::Net::HTTPServerRequest&, Poco::Net::HTTPServerResponse&);
};

#endif

