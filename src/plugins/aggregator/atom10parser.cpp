#include <QDomDocument>
#include <QDomElement>
#include <QList>
#include <QString>
#include "channel.h"
#include "item.h"
#include "atom10parser.h"

Atom10Parser::Atom10Parser ()
{
}

Atom10Parser& Atom10Parser::Instance ()
{
    static Atom10Parser inst;
    return inst;
}

bool Atom10Parser::CouldParse (const QDomDocument& doc) const
{
    QDomElement root = doc.documentElement ();
    return root.tagName () == "feed";
}

QList<Channel*> Atom10Parser::Parse (const QList<Channel*>& old, const QDomDocument& n) const
{
}

