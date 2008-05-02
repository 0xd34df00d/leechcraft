#ifndef TAGSCOMPLETER_H
#define TAGSCOMPLETER_H
#include <QCompleter>

class QLineEdit;

class TagsCompleter : public QCompleter
{
    Q_OBJECT
public:
    TagsCompleter (QLineEdit*, QObject* = 0);
    virtual QStringList splitPath (const QString&) const;
};

#endif

