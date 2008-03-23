#ifndef RSS20PARSER_H
#define RSS20PARSER_H
#include <QPair>
#include <QDateTime>
#include "parserfactory.h"
#include "parser.h"

class Item;

class RSS20Parser : public Parser
{
    Q_OBJECT

    friend class ParserFactory;

    RSS20Parser ();
public:
    static RSS20Parser& Instance ();
    virtual bool CouldParse (const QDomDocument&) const;
    virtual std::vector<boost::shared_ptr<Channel> > Parse (const std::vector<boost::shared_ptr<Channel> >&, const QDomDocument&) const;
private:
    std::vector<boost::shared_ptr<Channel> > Parse (const QDomDocument&) const;
    Item* ParseItem (const QDomElement&) const;
    QDateTime FromRFC822 (const QString&) const;
};

#endif

