/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QObject>
#include <QStringList>
#include <QMap>
#include <QSqlDatabase>
#include <util/db/oral/oralfwd.h>

class QDir;

namespace LC
{
namespace Util
{
	class DBLock;
}

namespace Snails
{
	class Account;
	struct MessageInfo;
	struct MessageBodies;

	class AccountDatabase
	{
		QSqlDatabase DB_;
	public:
		struct Message;
		struct Address;
		struct Attachment;

		struct MessageBodies;

		struct Folder;
		struct Msg2Folder;
		struct MsgHeader;
	private:
		Util::oral::ObjectInfo_ptr<Message> Messages_;
		Util::oral::ObjectInfo_ptr<Address> Addresses_;
		Util::oral::ObjectInfo_ptr<Attachment> Attachments_;

		Util::oral::ObjectInfo_ptr<MessageBodies> MessagesBodies_;

		Util::oral::ObjectInfo_ptr<Folder> Folders_;
		Util::oral::ObjectInfo_ptr<Msg2Folder> Msg2Folder_;
		Util::oral::ObjectInfo_ptr<MsgHeader> MsgHeader_;

		QMap<QStringList, int> KnownFolders_;
	public:
		AccountDatabase (const QDir&, const QByteArray&);

		Util::DBLock BeginTransaction ();

		QList<QByteArray> GetIDs (const QStringList& folder);
		std::optional<QByteArray> GetLastID (const QStringList& folder);
		int GetMessageCount (const QStringList& folder);
		int GetUnreadMessageCount (const QStringList& folder);
		int GetMessageCount ();

		QList<MessageInfo> GetMessageInfos (const QStringList& folder);
		std::optional<MessageInfo> GetMessageInfo (const QStringList& folder, const QByteArray& msgId);

		void AddMessage (const MessageInfo&);
		void RemoveMessage (const QByteArray& msgId, const QStringList& folder);

		void SaveMessageBodies (const QStringList& folder, const QByteArray& msgId, const Snails::MessageBodies&);
		std::optional<Snails::MessageBodies> GetMessageBodies (const QStringList& folder, const QByteArray& msgId);
		bool HasMessageBodies (const QStringList& folder, const QByteArray& msgId);

		std::optional<bool> IsMessageRead (const QByteArray& msgId, const QStringList& folder);
		void SetMessageRead (const QByteArray& msgId, const QStringList& folder, bool read);

		void SetMessageHeader (const QByteArray& msgId, const QByteArray& header);
		std::optional<QByteArray> GetMessageHeader (const QByteArray& uniqueMsgId) const;
		std::optional<QByteArray> GetMessageHeader (const QStringList& folderId, const QByteArray& msgId) const;

		std::optional<int> GetMsgTableId (const QByteArray& uniqueId);
		std::optional<int> GetMsgTableId (const QByteArray& msgId, const QStringList& folder);
	private:
		int AddMessageUnfoldered (const MessageInfo&);
		void AddMessageToFolder (int msgTableId, int folderTableId, const QByteArray& msgId);

		int AddFolder (const QStringList&);
		int GetFolder (const QStringList&) const;
		void LoadKnownFolders ();
	};
}
}
