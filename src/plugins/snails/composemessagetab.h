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

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "ui_composemessagetab.h"
#include "account.h"

class IEditorWidget;

namespace LeechCraft
{
namespace Snails
{
	class ComposeMessageTab : public QWidget
							, public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		static QObject *S_ParentPlugin_;
		static TabClassInfo S_TabClassInfo_;

		Ui::ComposeMessageTab Ui_;

		QToolBar *Toolbar_;
		QMenu *AccountsMenu_;
		QMenu *AttachmentsMenu_;

		QWidget *MsgEditWidget_;
		IEditorWidget *MsgEdit_;
	public:
		static void SetParentPlugin (QObject*);
		static void SetTabClassInfo (const TabClassInfo&);

		ComposeMessageTab (QWidget* = 0);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs();
		void Remove();
		QToolBar* GetToolBar() const;

		void SelectAccount (Account_ptr);
		void PrepareReply (Message_ptr);
	private slots:
		void handleSend ();
		void handleAddAttachment ();
		void handleRemoveAttachment ();
	signals:
		void removeTab (QWidget*);
	};
}
}
