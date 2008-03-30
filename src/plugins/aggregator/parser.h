#ifndef PARSER_H
#define PARSER_H
#include <QObject>
#include <QDomDocument>
#include <vector>
#include <boost/shared_ptr.hpp>

class Channel;

class Parser : public QObject
{
    Q_OBJECT
public:
    virtual bool CouldParse (const QDomDocument&) const = 0;
    virtual std::vector<boost::shared_ptr<Channel> > Parse (const std::vector<boost::shared_ptr<Channel> >&, const QDomDocument&) const = 0;
    std::vector<boost::shared_ptr<Channel> > Parse (const std::vector<boost::shared_ptr<Channel> >& o, const QByteArray& n)
    {
        QDomDocument newd;
        newd.setContent (n);
        return Parse (o, newd);
    }
protected:
    QString UnescapeHTML (const QString&) const;
};

#endif

