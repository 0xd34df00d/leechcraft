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

#include "chathistorywidget.h"
#include "chathistory.h"

namespace LeechCraft
{
namespace Azoth
{
namespace ChatHistory
{
	Plugin *ChatHistoryWidget::S_ParentMultiTabs_ = 0;

	void ChatHistoryWidget::SetParentMultiTabs (Plugin *ch)
	{
		S_ParentMultiTabs_ = ch;
	}

	ChatHistoryWidget::ChatHistoryWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
	}
	
	void ChatHistoryWidget::Remove ()
	{
		emit removeSelf (this);
	}
	
	QToolBar* ChatHistoryWidget::GetToolBar () const
	{
		return 0;
	}
	
	void ChatHistoryWidget::NewTabRequested ()
	{
		S_ParentMultiTabs_->newTabRequested ();
	}
	
	QObject* ChatHistoryWidget::ParentMultiTabs () const
	{
		return S_ParentMultiTabs_;
	}
	
	QList<QAction*> ChatHistoryWidget::GetTabBarContextMenuActions () const
	{
		return QList<QAction*> ();
	}
}
}
}
