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

#include "dashtab.h"
#include <QGraphicsView>
#include <QVBoxLayout>

namespace LeechCraft
{
namespace BlackDash
{
	QObject *DashTab::S_ParentPlugin_ = 0;

	void DashTab::SetParentPlugin (QObject *parent)
	{
		S_ParentPlugin_ = parent;
	}
	
	TabClassInfo DashTab::GetStaticTabClassInfo ()
	{
		TabClassInfo info =
		{
			"org.LeechCraft.BlackDash.Dash",
			tr ("Dashboard"),
			tr ("Dashboard for widgets and shortcuts"),
			QIcon (),
			70,
			TFOpenableByRequest
		};
		return info;
	}

	DashTab::DashTab (QWidget *parent)
	: QWidget (parent)
	, View_ (new QGraphicsView)
	{
		setLayout (new QVBoxLayout ());
		layout ()->addWidget (View_);
	}
	
	TabClassInfo DashTab::GetTabClassInfo () const
	{
		return GetStaticTabClassInfo ();
	}
	
	QObject* DashTab::ParentMultiTabs ()
	{
		return S_ParentPlugin_;
	}
	
	QToolBar* DashTab::GetToolBar () const
	{
		return 0;
	}

	void DashTab::Remove ()
	{
		emit removeTab (this);
	}
}
}
