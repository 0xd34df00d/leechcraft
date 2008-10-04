#ifndef ATOM03PARSER_H
#define ATOM03PARSER_H
#include <QPair>
#include <QDateTime>
#include "parserfactory.h"
#include "atomparser.h"

class Atom03Parser : public AtomParser
{
    friend class ParserFactory;

    Atom03Parser ();
public:
    static Atom03Parser& Instance ();
    virtual bool CouldParse (const QDomDocument&) const;
private:
	Feed::channels_container_t Parse (const QDomDocument&) const;
    Item* ParseItem (const QDomElement&) const;
};

#endif

