/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include <interfaces/iwkfontssettable.h>
#include "ui_composemessagetab.h"
#include "account.h"

class IEditorWidget;

namespace LC
{
namespace Snails
{
	class AccountsManager;
	class MsgTemplatesManager;
	class AttachmentsFetcher;
	struct OutgoingMessage;

	enum class MsgType;

	class ComposeMessageTab : public QWidget
							, public ITabWidget
							, public IWkFontsSettable
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IWkFontsSettable)

		static QObject *S_ParentPlugin_;
		static TabClassInfo S_TabClassInfo_;

		Ui::ComposeMessageTab Ui_;

		const AccountsManager * const AccsMgr_;
		const MsgTemplatesManager * const TemplatesMgr_;

		QToolBar *Toolbar_;
		QMenu *AccountsMenu_;
		QMenu *AttachmentsMenu_;
		QMenu *EditorsMenu_;

		QList<QByteArray> OrigReferences_;
		QByteArray OrigMessageId_;

		std::shared_ptr<AttachmentsFetcher> LinkedAttachmentsFetcher_;
	public:
		static void SetParentPlugin (QObject*);
		static void SetTabClassInfo (const TabClassInfo&);

		ComposeMessageTab (const AccountsManager*, const MsgTemplatesManager*, QWidget* = nullptr);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;

		QObject* GetQObject ();
		void SetFontFamily (FontFamily, const QFont&);
		void SetFontSize (FontSize, int);

		void SelectAccount (const Account_ptr&);
		void PrepareLinked (MsgType, const MessageInfo&, const MessageBodies&);
	private:
		void PrepareLinkedEditor (const MessageBodies&);
		void PrepareLinkedBody (MsgType, const MessageInfo&, const MessageBodies&);
		void CopyAttachments (const MessageInfo&);

		void SetupToolbar ();
		void SetupEditors ();

		void SetMessageReferences (OutgoingMessage&) const;

		Account* GetSelectedAccount () const;

		void AppendAttachment (const QString& path, const QString& descr);

		void AddAttachments (OutgoingMessage&);
		void Send (Account*, const OutgoingMessage&);
	private slots:
		void handleSend ();
		void handleAddAttachment ();

		void handleEditorChanged (IEditorWidget*, IEditorWidget*);
	signals:
		void removeTab (QWidget*);
	};
}
}
