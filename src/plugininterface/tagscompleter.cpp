#include "tagscompleter.h"
#include <QtDebug>
#include <QWidget>
#include <QStringList>
#include <QLineEdit>
#include "tagslineedit.h"

using namespace LeechCraft::Util;

TagsCompleter::TagsCompleter (TagsLineEdit *toComplete, QObject *parent)
: QCompleter (parent)
{
    toComplete->SetCompleter (this);
}

QStringList TagsCompleter::splitPath (const QString& path) const
{
	return path.split (' ', QString::SkipEmptyParts);
}

