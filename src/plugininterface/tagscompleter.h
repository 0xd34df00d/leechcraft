#ifndef TAGSCOMPLETER_H
#define TAGSCOMPLETER_H
#include <QCompleter>

class TagsCompleter : public QCompleter
{
    Q_OBJECT
public:
    TagsCompleter (QObject *parent = 0);
    virtual QStringList splitPath (const QString&) const;
};

#endif

