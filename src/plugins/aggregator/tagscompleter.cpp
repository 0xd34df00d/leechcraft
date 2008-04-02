#include <QtDebug>
#include <QStringList>
#include "tagscompleter.h"

TagsCompleter::TagsCompleter (QObject *parent)
: QCompleter (parent)
{
}

QStringList TagsCompleter::splitPath (const QString& path) const
{
    return path.split (' ', QString::SkipEmptyParts);
}

