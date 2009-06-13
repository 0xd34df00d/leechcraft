#include <QLineEdit>
#include <QtDebug>
#include "tagscompletionmodel.h"

using namespace LeechCraft::Util;

TagsCompletionModel::TagsCompletionModel (QObject *parent)
: QStringListModel (parent)
{
}

void TagsCompletionModel::UpdateTags (const QStringList& newTags)
{
	QStringList oldTags = stringList ();
	for (int i = 0; i < newTags.size (); ++i)
		if (!oldTags.contains (newTags.at (i)))
			oldTags.append (newTags.at (i));

	setStringList (oldTags);
	emit tagsUpdated (oldTags);
}

