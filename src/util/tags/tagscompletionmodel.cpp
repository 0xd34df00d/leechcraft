/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QLineEdit>
#include <QtDebug>
#include "tagscompletionmodel.h"

namespace LC::Util
{
	void TagsCompletionModel::UpdateTags (const QStringList& newTags)
	{
		auto oldTags = stringList ();
		for (int i = 0; i < newTags.size (); ++i)
			if (!oldTags.contains (newTags.at (i)))
				oldTags.append (newTags.at (i));

		setStringList (oldTags);
		emit tagsUpdated (oldTags);
	}
}
