/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsasker.h"
#include <util/tags/tagscompleter.h>

namespace LC::SeekThru
{
	TagsAsker::TagsAsker (const QString& text, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		new Util::TagsCompleter (Ui_.Tags_);
		Ui_.Tags_->AddSelector ();
		Ui_.Tags_->setText (text);
	}

	QString TagsAsker::GetTags () const
	{
		return Ui_.Tags_->text ();
	}
}
