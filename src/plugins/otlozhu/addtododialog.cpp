/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "addtododialog.h"
#include <algorithm>
#include <interfaces/core/itagsmanager.h>
#include <util/tags/tagscompleter.h>
#include "core.h"

namespace LeechCraft
{
namespace Otlozhu
{
	AddTodoDialog::AddTodoDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		new Util::TagsCompleter (Ui_.Tags_, Ui_.Tags_);
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
