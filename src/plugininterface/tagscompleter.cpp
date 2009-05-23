#include "tagscompleter.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <QtDebug>
#include <QWidget>
#include <QStringList>
#include <QLineEdit>
#include "tagslineedit.h"

using namespace LeechCraft::Util;

QAbstractItemModel *LeechCraft::Util::TagsCompleter::CompletionModel_ = 0;

TagsCompleter::TagsCompleter (TagsLineEdit *toComplete, QObject *parent)
: QCompleter (parent)
{
	setCompletionRole (Qt::DisplayRole);
	setModel (CompletionModel_);
    toComplete->SetCompleter (this);
}

QStringList TagsCompleter::splitPath (const QString& string) const
{
	QStringList splitted = string.split (";", QString::SkipEmptyParts);
	QStringList result;
	Q_FOREACH (QString s, splitted)
		result << s.trimmed ();
	return result;
}

