/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QList>
#include <QPair>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iupdatablechatentry.h>

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class VkAccount;
	class VkMessage;
	struct MessageInfo;
	struct FullMessageInfo;

	class EntryBase : public QObject
					, public ICLEntry
					, public IUpdatableChatEntry
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ICLEntry
				LC::Azoth::IUpdatableChatEntry)
	protected:
		VkAccount * const Account_;
		QList<VkMessage*> Messages_;

		bool HasUnread_ = false;
	public:
		EntryBase (VkAccount*);

		virtual void Send (VkMessage*) = 0;
		void Store (VkMessage*);

		QObject* GetQObject () override;
		IAccount* GetParentAccount () const override;

		IMessage* CreateMessage (IMessage::Type type, const QString& variant, const QString& body) override;
		QList<IMessage*> GetAllMessages () const override;
		void PurgeMessages (const QDateTime& before) override;

		void MarkMsgsRead () override;
	protected:
		void HandleAttaches (VkMessage*, const MessageInfo&, const FullMessageInfo&);
	private:
		void HandleFullMessageInfo (const FullMessageInfo&, VkMessage*);
		void PerformReplacements (QList<QPair<QString, QString>>, QString&);
	signals:
		void gotMessage (QObject*) override;
		void statusChanged (const EntryStatus&, const QString&) override;
		void availableVariantsChanged (const QStringList&) override;
		void nameChanged (const QString&) override;
		void groupsChanged (const QStringList&) override;
		void chatPartStateChanged (const ChatPartState&, const QString&) override;
		void permsChanged () override;
		void entryGenerallyChanged () override;

		void performJS (const QString&) override;
	};
}
}
}
