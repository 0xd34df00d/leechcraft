/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "storage.h"
#include <stdexcept>
#include <QApplication>
#include <QThread>
#include <QSqlQuery>
#include <QSqlError>
#include <QDataStream>
#include <util/db/dblock.h>
#include <util/sys/paths.h>
#include "xmlsettingsmanager.h"
#include "account.h"
#include "accountdatabase.h"
#include "messageinfo.h"
#include "messagebodies.h"

namespace LC
{
namespace Snails
{
	namespace
	{
		template<typename T>
		QByteArray Serialize (const T& t)
		{
			QByteArray result;
			QDataStream stream (&result, QIODevice::WriteOnly);
			stream << t;
			return result;
		}
	}

	Storage::Storage (QObject *parent)
	: QObject (parent)
	, CachedThread_ { QThread::currentThreadId () }
	{
		SDir_ = Util::CreateIfNotExists ("snails/storage");
	}

	void Storage::SaveMessageInfos (Account *acc, const QList<MessageInfo>& infos)
	{
		const auto& base = BaseForAccount (acc);
		for (const auto& info : infos)
			base->AddMessage (info);
	}

	QList<MessageInfo> Storage::GetMessageInfos (Account *acc, const QStringList& folder)
	{
		return BaseForAccount (acc)->GetMessageInfos (folder);
	}

	std::optional<MessageInfo> Storage::GetMessageInfo (Account *acc, const QStringList& folder, const QByteArray& msgId)
	{
		return BaseForAccount (acc)->GetMessageInfo (folder, msgId);
	}

	void Storage::SaveMessageBodies (Account *acc,
			const QStringList& folder,
			const QByteArray& msgId,
			const MessageBodies& bodies)
	{
		BaseForAccount (acc)->SaveMessageBodies (folder, msgId, bodies);
	}

	std::optional<MessageBodies> Storage::GetMessageBodies (Account *acc,
			const QStringList& folder, const QByteArray& msgId)
	{
		return BaseForAccount (acc)->GetMessageBodies (folder, msgId);
	}

	bool Storage::HasMessageBodies (Account *acc, const QStringList& folder, const QByteArray& msgId)
	{
		return BaseForAccount (acc)->HasMessageBodies (folder, msgId);
	}

	QList<QByteArray> Storage::LoadIDs (Account *acc, const QStringList& folder)
	{
		return BaseForAccount (acc)->GetIDs (folder);
	}

	std::optional<QByteArray> Storage::GetLastID (Account *acc, const QStringList& folder)
	{
		return BaseForAccount (acc)->GetLastID (folder);
	}

	void Storage::RemoveMessage (Account *acc, const QStringList& folder, const QByteArray& id)
	{
		BaseForAccount (acc)->RemoveMessage (id, folder);
	}

	int Storage::GetNumMessages (Account *acc, const QStringList& folder)
	{
		return BaseForAccount (acc)->GetMessageCount (folder);
	}

	int Storage::GetNumUnread (Account *acc, const QStringList& folder)
	{
		return BaseForAccount (acc)->GetUnreadMessageCount (folder);
	}

	bool Storage::IsMessageRead (Account *acc, const QStringList& folder, const QByteArray& id)
	{
		return BaseForAccount (acc)->IsMessageRead (id, folder).value ();
	}

	void Storage::SetMessagesRead (Account *acc,
			const QStringList& folder, const QList<QByteArray>& folderIds, bool read)
	{
		if (folderIds.isEmpty ())
			return;

		auto base = BaseForAccount (acc);

		qDebug () << "SetRead" << folderIds.size ();
		auto ts = base->BeginTransaction ();
		for (const auto& id : folderIds)
			base->SetMessageRead (id, folder, read);
		ts.Good ();
		qDebug () << "done";
	}

	QDir Storage::DirForAccount (const Account *acc) const
	{
		const QByteArray& id = acc->GetID ().toHex ();

		QDir dir = SDir_;
		if (!dir.exists (id))
			dir.mkdir (id);
		if (!dir.cd (id))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cd into"
					<< dir.filePath (id);
			throw std::runtime_error ("Unable to cd to the dir");
		}

		return dir;
	}

	AccountDatabase_ptr Storage::BaseForAccount (const Account *acc)
	{
		const auto isCachedThread = QThread::currentThreadId () == CachedThread_;

		if (isCachedThread && AccountBases_.contains (acc))
			return AccountBases_ [acc];

		const auto& dir = DirForAccount (acc);
		const auto& base = std::make_shared<AccountDatabase> (dir, acc->GetID ());
		if (isCachedThread)
			AccountBases_ [acc] = base;
		return base;
	}
}
}
