#ifndef ATOM10PARSER_H
#define ATOM10PARSER_H
#include <QPair>
#include <QDateTime>
#include "parserfactory.h"
#include "parser.h"

class Item;

class Atom10Parser : public Parser
{
    Q_OBJECT

    friend class ParserFactory;

    Atom10Parser ();
public:
    static Atom10Parser& Instance ();
    virtual bool CouldParse (const QDomDocument&) const;
    virtual std::vector<boost::shared_ptr<Channel> > Parse (const std::vector<boost::shared_ptr<Channel> >&, const QDomDocument&) const;
private:
    std::vector<boost::shared_ptr<Channel> > Parse (const QDomDocument&) const;
    Item* ParseItem (const QDomElement&) const;
    QDateTime FromRFC3339 (const QString&) const;
    QString GetLink (const QDomElement&) const;
};

#endif

