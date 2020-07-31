/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <optional>
#include <QObject>
#include <QDir>
#include <QSettings>
#include <QHash>
#include <QSet>

namespace LC
{
namespace Snails
{
	class Account;

	class AccountDatabase;
	using AccountDatabase_ptr = std::shared_ptr<AccountDatabase>;

	struct MessageInfo;
	struct MessageBodies;

	class Storage : public QObject
	{
		QDir SDir_;

		QHash<const Account*, AccountDatabase_ptr> AccountBases_;
		const Qt::HANDLE CachedThread_;
	public:
		Storage (QObject* = nullptr);

		AccountDatabase_ptr BaseForAccount (const Account*);

		void SaveMessageInfos (Account*, const QList<MessageInfo>&);
		QList<MessageInfo> GetMessageInfos (Account*, const QStringList& folder);
		std::optional<MessageInfo> GetMessageInfo (Account*, const QStringList& folder, const QByteArray& msgId);

		void SaveMessageBodies (Account*, const QStringList& folder, const QByteArray& msgId, const MessageBodies&);
		std::optional<MessageBodies> GetMessageBodies (Account*, const QStringList& folder, const QByteArray& msgId);
		bool HasMessageBodies (Account*, const QStringList& folder, const QByteArray& msgId);

		QList<QByteArray> LoadIDs (Account*, const QStringList& folder);
		std::optional<QByteArray> GetLastID (Account*, const QStringList& folder);
		void RemoveMessage (Account*, const QStringList&, const QByteArray&);

		int GetNumMessages (Account*, const QStringList& folder);
		int GetNumUnread (Account*, const QStringList& folder);

		bool IsMessageRead (Account*, const QStringList& folder, const QByteArray&);
		void SetMessagesRead (Account*, const QStringList& folder, const QList<QByteArray>& folderIds, bool read);
	private:
		QDir DirForAccount (const Account*) const;
	};
}
}
