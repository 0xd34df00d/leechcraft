/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagscompleter.h"
#include <QtDebug>
#include <QStringList>
#include "tagslineedit.h"
#include "util.h"

namespace LC::Util
{
	QAbstractItemModel *TagsCompleter::CompletionModel_ = nullptr;

	TagsCompleter::TagsCompleter (TagsLineEdit *toComplete)
	: QCompleter (toComplete)
	, Edit_ (toComplete)
	{
		setCompletionRole (Qt::DisplayRole);
		setModel (CompletionModel_);
		toComplete->SetCompleter (this);
	}

	void TagsCompleter::OverrideModel (QAbstractItemModel *model)
	{
		setModel (model);
	}

	QStringList TagsCompleter::splitPath (const QString& string) const
	{
		return SplitTags (string, Edit_->GetSeparator ());
	}
}
