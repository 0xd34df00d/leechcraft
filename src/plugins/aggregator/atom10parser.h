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
    virtual Feed::channels_container_t Parse (const Feed::channels_container_t&, const QDomDocument&) const;
private:
	Feed::channels_container_t Parse (const QDomDocument&) const;
    Item* ParseItem (const QDomElement&) const;
    QDateTime FromRFC3339 (const QString&) const;
    QString GetLink (const QDomElement&) const;
};

#endif

