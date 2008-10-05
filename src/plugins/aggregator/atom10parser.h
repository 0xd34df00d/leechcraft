#ifndef ATOM10PARSER_H
#define ATOM10PARSER_H
#include <QPair>
#include <QDateTime>
#include "parserfactory.h"
#include "atomparser.h"

class Atom10Parser : public AtomParser
{
    friend class ParserFactory;

    Atom10Parser ();
public:
    static Atom10Parser& Instance ();
    virtual bool CouldParse (const QDomDocument&) const;
private:
	channels_container_t Parse (const QDomDocument&) const;
    Item* ParseItem (const QDomElement&) const;
};

#endif

