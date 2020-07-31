/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>

namespace LC
{
namespace Snails
{
	class MailModel;
	class Account;
	class Storage;
	class MessageListActionsManager;

	struct MessageInfo;

	class MailModelsManager : public QObject
	{
		Account * const Acc_;
		Storage * const Storage_;
		MessageListActionsManager * const MsgListActionsMgr_;

		QList<MailModel*> Models_;
	public:
		MailModelsManager (Account*, Storage*);

		std::unique_ptr<MailModel> CreateModel ();

		void ShowFolder (const QStringList&, MailModel*);

		void Append (const QList<MessageInfo>&);
		void Remove (const QList<QByteArray>&);

		void UpdateReadStatus (const QStringList& folderId, const QList<QByteArray>& msgIds, bool read);
	};
}
}
