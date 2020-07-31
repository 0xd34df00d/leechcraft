/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addtododialog.h"
#include <algorithm>
#include <interfaces/core/itagsmanager.h>
#include <util/tags/tagscompleter.h>
#include "core.h"

namespace LC
{
namespace Otlozhu
{
	AddTodoDialog::AddTodoDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		new Util::TagsCompleter (Ui_.Tags_);
		Ui_.Tags_->AddSelector ();
	}

	TodoItem_ptr AddTodoDialog::GetItem () const
	{
		TodoItem_ptr item (new TodoItem);
		item->SetTitle (GetTitle ());
		item->SetTagIDs (GetTags ());
		return item;
	}

	QString AddTodoDialog::GetTitle () const
	{
		return Ui_.Title_->text ();
	}

	QStringList AddTodoDialog::GetTags () const
	{
		return Core::Instance ().GetProxy ()->
				GetTagsManager ()->SplitToIDs (Ui_.Tags_->text ());
	}
}
}
