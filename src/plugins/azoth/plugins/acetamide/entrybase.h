/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QVariant>
#include <interfaces/azoth/iclentry.h>
#include "localtypes.h"

class QImage;

namespace LC::Azoth::Acetamide
{
	class IrcAccount;
	class IrcMessage;
	class VCardDialog;

	class EntryBase : public QObject
					, public ICLEntry
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ICLEntry)
	protected:
		QList<IMessage*> AllMessages_;
		EntryStatus CurrentStatus_;
		QList<QAction*> Actions_;

		IrcAccount * const Account_;
		VCardDialog *VCardDialog_ = nullptr;

	public:
		explicit EntryBase (IrcAccount*);
		~EntryBase () override;

		QObject* GetQObject () override;
		QList<IMessage*> GetAllMessages () const override;
		void PurgeMessages (const QDateTime&) override;
		void SetChatPartState (ChatPartState, const QString&) override;
		EntryStatus GetStatus (const QString&) const override;
		QList<QAction*> GetActions () const override;
		void ShowInfo () override;
		QMap<QString, QVariant> GetClientInfo (const QString&) const override;

		void MarkMsgsRead () override;
		void ChatTabClosed () override;

		void HandleMessage (IrcMessage*);
		void SetStatus (const EntryStatus&);
		void SetAvatar (const QByteArray&);
		void SetAvatar (const QImage&);
		void SetInfo (const WhoIsMessage& msg);

	signals:
		void gotMessage (QObject*) override;
		void statusChanged (const EntryStatus&, const QString&) override;
		void availableVariantsChanged (const QStringList&) override;
		void nameChanged (const QString&) override;
		void groupsChanged (const QStringList&) override;
		void chatPartStateChanged (const ChatPartState&, const QString&) override;
		void permsChanged () override;
		void entryGenerallyChanged () override;
	};
}
