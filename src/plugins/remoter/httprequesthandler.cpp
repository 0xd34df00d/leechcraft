#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/MultipartReader.h>
#include <Poco/Net/NetException.h>
#include <QtDebug>
#include <QStringList>
#include <iostream>
#include <functional>
#include <string>
#include "core.h"
#include "httprequesthandler.h"

void HTTPRequestHandler::handleRequest (Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
    if (!Core::Instance ().IsAuthorized (request))
    {
        response.requireAuthentication ("LeechCraft Remoter");
        response.setContentType ("text/html");
        response.send () << "<html><head><title>401 Not Authorized</title></head><body>Sorry, you are not authorized, better go away, or I'll call police!</body></html>";
        return;
    }

    QString fullUri = QString::fromStdString (request.getURI ());
    QMap<QString, QString> query;
    if (fullUri.contains ('?'))
    {
        QStringList parts = fullUri.split ('?', QString::SkipEmptyParts);
        fullUri = parts [0];
        if (parts.size () > 1)
        {
            QStringList pairs = parts [1].split ('&', QString::SkipEmptyParts);
            for (int i = 0; i < pairs.size (); ++i)
            {
                QStringList tmp = pairs.at (i).split ('=');
                if (tmp.size () == 2)
                    query [tmp.at (0)] = tmp.at (1);
            }
        }
    }

    QList<Core::PostEntity> postData;
    Poco::Net::MultipartReader mreader (request.stream ());
    do 
    {
        Poco::Net::MessageHeader h;
        try
        {
            mreader.nextPart (h);
        }
        catch (...)
        {
            break;
        }
        std::string str;
        std::getline (mreader.stream (), str, static_cast<char> (0));
        Core::PostEntity entity;
        entity.Data_ = str.c_str ();
        for (Poco::Net::NameValueCollection::ConstIterator i = h.begin (), end = h.end (); i != end; ++i)
        {
            std::string key;
            std::transform (i->first.begin (), i->first.end (), std::back_inserter (key), std::ptr_fun (tolower));
            if (key == "content-disposition")
            {
                std::string value;
                Poco::Net::NameValueCollection parms;
                Poco::Net::MessageHeader::splitParameters (i->second, value, parms);
                for (Poco::Net::NameValueCollection::ConstIterator j = parms.begin (), end = parms.end (); j != end; ++j)
                    entity.Metadata_ [QString::fromStdString (j->first)] = QString::fromStdString (j->second);
            }
        }
        postData << entity;
    } while (mreader.hasNextPart ());

    Reply rep = Core::Instance ().GetReplyFor (fullUri, query, postData);
    if (rep.State_ == StateOK)
    {
        response.setContentType (rep.Type_.toStdString ());
        std::ostream& ostr = response.send ();
        ostr << rep.Data_.toUtf8 ().constData ();
    }
    else if (rep.State_ == StateFound)
    {
        QString host;
        try
        {
            host = QString::fromStdString (request.getHost ());
        }
        catch (...)
        {
        }
        response.redirect (rep.RedirectTo_.toStdString ());
    }
}

