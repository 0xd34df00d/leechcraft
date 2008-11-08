#include <QtDebug>
#include <QWidget>
#include <QStringList>
#include <QLineEdit>
#include "tagscompleter.h"

TagsCompleter::TagsCompleter (QLineEdit *toComplete, QObject *parent)
: QCompleter (parent)
{
    toComplete->setCompleter (this);
	connect (this,
			SIGNAL (activated (const QString&)),
			toComplete,
			SLOT (complete (const QString&)));
}

QStringList TagsCompleter::splitPath (const QString& path) const
{
    return path.split (' ', QString::SkipEmptyParts);
}

