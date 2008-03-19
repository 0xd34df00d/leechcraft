#ifndef PARSER_H
#define PARSER_H
#include <QObject>
#include <QDomDocument>

class Channel;

class Parser : public QObject
{
    Q_OBJECT
public:
    virtual bool CouldParse (const QDomDocument&) const = 0;
    virtual QList<Channel*> Parse (const QList<Channel*>&, const QDomDocument&) const = 0;
    QList<Channel*> Parse (const QList<Channel*>& o, const QByteArray& n)
    {
        QDomDocument newd;
        newd.setContent (n);
        return Parse (o, newd);
    }
protected:
    QString UnescapeHTML (const QString&) const;
};

#endif

