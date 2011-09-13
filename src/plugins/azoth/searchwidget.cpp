/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "searchwidget.h"

namespace LeechCraft
{
namespace Azoth
{
	QObject* SearchWidget::S_ParentMultiTabs_ = 0;

	void SearchWidget::SetParentMultiTabs (QObject *parent)
	{
		S_ParentMultiTabs_ = parent;
	}

	SearchWidget::SearchWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
	}

	TabClassInfo SearchWidget::GetTabClassInfo () const
	{
		TabClassInfo searchTab =
		{
			"Search",
			tr ("Search"),
			tr ("A search tab allows to search within IM services"),
			QIcon (),
			55,
			TFOpenableByRequest
		};
		return searchTab;
	}

	QObject* SearchWidget::ParentMultiTabs ()
	{
		return S_ParentMultiTabs_;
	}

	void SearchWidget::Remove ()
	{
		emit removeTab (this);
	}

	QToolBar* SearchWidget::GetToolBar () const
	{
		return 0;
	}
}
}
