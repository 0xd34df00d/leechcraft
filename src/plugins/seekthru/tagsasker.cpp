/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsasker.h"
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include <util/tags/tagseditoraddon.h>
#include "core.h"

namespace LC::SeekThru
{
	TagsAsker::TagsAsker (const QString& text, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		new Util::TagsEditorAddon { *Ui_.Tags_, *Core::Instance ().GetProxy ()->GetTagsManager () };
		Ui_.Tags_->setText (text);
	}

	QString TagsAsker::GetTags () const
	{
		return Ui_.Tags_->text ();
	}
}
