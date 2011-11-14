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

#include "composemessagetab.h"
#include <QToolBar>

namespace LeechCraft
{
namespace Snails
{
	QObject *ComposeMessageTab::S_ParentPlugin_ = 0;
	TabClassInfo ComposeMessageTab::S_TabClassInfo_;

	void ComposeMessageTab::SetParentPlugin (QObject *obj)
	{
		S_ParentPlugin_ = obj;
	}

	void ComposeMessageTab::SetTabClassInfo (const TabClassInfo& info)
	{
		S_TabClassInfo_ = info;
	}

	ComposeMessageTab::ComposeMessageTab (QWidget *parent)
	: QWidget (parent)
	, Toolbar_ (new QToolBar (tr ("Compose tab bar")))
	{
		Ui_.setupUi (this);

		QAction *send = new QAction (tr ("Send"));
		send->setProperty ("ActionIcon", "msgsend");
		connect (send,
				SIGNAL (triggered ()),
				this,
				SLOT (handleSend ()));

		Toolbar_->addAction (send);
	}

	TabClassInfo ComposeMessageTab::GetTabClassInfo () const
	{
		return S_TabClassInfo_;
	}

	QObject* ComposeMessageTab::ParentMultiTabs ()
	{
		return S_ParentPlugin_;
	}

	void ComposeMessageTab::Remove ()
	{
		emit removeTab (this);
	}

	QToolBar* ComposeMessageTab::GetToolBar () const
	{
		return Toolbar_;
	}

	void ComposeMessageTab::handleSend ()
	{

	}
}
}
