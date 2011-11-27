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

#include "sbwidget.h"
#include <QToolButton>
#include <util/flowlayout.h>

namespace LeechCraft
{
namespace Sidebar
{
	SBWidget::SBWidget (QWidget *parent)
	: QWidget (parent)
	, TrayLay_ (new Util::FlowLayout (1, 1, 1))
	, IconSize_ (QSize (48, 48))
	{
		Ui_.setupUi (this);
		static_cast<QVBoxLayout*> (layout ())->addLayout (TrayLay_);

		setMaximumWidth (50);
	}

	void SBWidget::AddTabOpenAction (QAction *act)
	{
		auto tb = new QToolButton;
		tb->setIconSize (IconSize_);
		tb->setDefaultAction (act);
		Ui_.PluginButtonsLay_->addWidget (tb);
	}

	void SBWidget::AddQLAction (QAction *act)
	{
		auto tb = new QToolButton;
		tb->setIconSize (IconSize_);
		tb->setDefaultAction (act);
		Ui_.QLLay_->addWidget (tb);
	}

	void SBWidget::AddCurTabAction (QAction *act)
	{
		auto tb = new QToolButton;
		tb->setIconSize (IconSize_);
		tb->setDefaultAction (act);
		Ui_.TabsLay_->addWidget (tb);

		CurTab2Button_ [act] = tb;
	}

	void SBWidget::RemoveCurTabAction (QAction *act)
	{
		delete CurTab2Button_.take (act);
	}

	void SBWidget::AddTrayAction (QAction *act)
	{
		connect (act,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleTrayActDestroyed ()));

		auto tb = new QToolButton;
		tb->setIconSize (IconSize_ / 2.1);
		tb->setDefaultAction (act);
		TrayAct2Button_ [act] = tb;

		TrayLay_->addWidget (tb);
	}

	void SBWidget::handleTrayActDestroyed ()
	{
		delete TrayAct2Button_.take (static_cast<QAction*> (sender ()));
	}
}
}
