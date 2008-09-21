#ifndef PARSER_H
#define PARSER_H
#include <QObject>
#include <QDomDocument>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "feed.h"

struct Channel;

class Parser : public QObject
{
    Q_OBJECT
public:
    virtual bool CouldParse (const QDomDocument&) const = 0;
    virtual Feed::channels_container_t Parse (const Feed::channels_container_t&, const QDomDocument&) const = 0;
	Feed::channels_container_t Parse (const Feed::channels_container_t& o, const QByteArray& n)
    {
        QDomDocument newd;
        newd.setContent (n);
        return Parse (o, newd);
    }
protected:
    QString UnescapeHTML (const QString&) const;
};

#endif

