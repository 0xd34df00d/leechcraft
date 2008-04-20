#ifndef HTTPREQUESTHANDLERFACTORY_H
#define HTTPREQUESTHANDLERFACTORY_H
#include <Poco/Net/HTTPRequestHandlerFactory.h>

namespace Poco
{
    namespace Net
    {
        class HTTPRequestHandler;
        class HTTPServerRequest;
    };
};

class HTTPRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
{
public:
    virtual Poco::Net::HTTPRequestHandler* createRequestHandler (const Poco::Net::HTTPServerRequest&);
};

#endif

