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

#include "juffedwidget.h"

namespace LeechCraft
{
namespace JuffEd
{
	QObject *JuffEdWidget::S_ParentMultiTabs_ = 0;
	
	void JuffEdWidget::SetParentMultiTabs (QObject *obj)
	{
		S_ParentMultiTabs_ = obj;
	}

	JuffEdWidget::JuffEdWidget (QWidget *parent)
	: QWidget (parent)
	{
	}
	
	void JuffEdWidget::Remove ()
	{
	}
	
	QToolBar* JuffEdWidget::GetToolBar () const
	{
		return 0;
	}

	QObject* JuffEdWidget::ParentMultiTabs ()
	{
		return S_ParentMultiTabs_;
	}
	
	QList<QAction*> JuffEdWidget::GetTabBarContextMenuActions () const
	{
		return QList<QAction*> ();
	}
}
}
