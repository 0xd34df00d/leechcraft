#ifndef PARSER_H
#define PARSER_H
#include <QObject>
#include <QDomDocument>
#include "item.h"

class Parser : public QObject
{
    Q_OBJECT
public:
    virtual bool CouldParse (const QDomDocument&) const = 0;
    virtual QList<Item> Parse (const QDomDocument&, const QDomDocument&) const = 0;
    QList<Item> Parse (const QByteArray& o, const QByteArray& n)
    {
        QDomDocument old, newd;
        old.setContent (o);
        newd.setContent (n);
        return Parse (old, newd);
    }
};

#endif

