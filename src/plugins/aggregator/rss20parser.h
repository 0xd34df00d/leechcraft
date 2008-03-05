#ifndef RSS20PARSER_H
#define RSS20PARSER_H
#include <QPair>
#include "parserfactory.h"
#include "parser.h"
#include "item.h"
#include "channel.h"

class RSS20Parser : public Parser
{
    Q_OBJECT

    friend class ParserFactory;

    RSS20Parser ();
public:
    static RSS20Parser& Instance ();
    virtual bool CouldParse (const QDomDocument&) const;
    virtual QList<Item> Parse (const QDomDocument&, const QDomDocument&) const;
private:
    QPair<QList<Channel>, QList<Item> > Parse (const QDomDocument&) const;
    Item ParseItem (const QDomElement&) const;
};

#endif

