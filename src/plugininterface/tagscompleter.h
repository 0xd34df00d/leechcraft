#ifndef TAGSCOMPLETER_H
#define TAGSCOMPLETER_H
#include <QCompleter>
#include "config.h"

class QLineEdit;

class TagsCompleter : public QCompleter
{
    Q_OBJECT
public:
    LEECHCRAFT_API TagsCompleter (QLineEdit*, QObject* = 0);
    LEECHCRAFT_API virtual QStringList splitPath (const QString&) const;
};

#endif

