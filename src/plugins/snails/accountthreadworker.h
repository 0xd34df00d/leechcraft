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

#include <QObject>
#include <vmime/net/session.hpp>
#include <vmime/net/message.hpp>
#include <vmime/net/folder.hpp>
#include <vmime/net/store.hpp>
#include <interfaces/structures.h>
#include "progresslistener.h"
#include "message.h"
#include "account.h"

namespace LeechCraft
{
namespace Snails
{
	class Account;

	class AccountThreadWorker : public QObject
	{
		Q_OBJECT

		Account *A_;
		vmime::ref<vmime::net::session> Session_;
		vmime::ref<vmime::net::store> CachedStore_;
		QHash<QStringList, vmime::ref<vmime::net::folder>> CachedFolders_;
		QTimer *DisconnectTimer_;

		QHash<QStringList, QHash<QByteArray, int>> SeqCache_;
	public:
		AccountThreadWorker (Account*);
	private:
		vmime::ref<vmime::net::store> MakeStore ();
		vmime::ref<vmime::net::transport> MakeTransport ();

		vmime::ref<vmime::net::folder> GetFolder (const QStringList& folder, int mode);

		Message_ptr FromHeaders (const vmime::ref<vmime::net::message>&) const;
		void FetchMessagesPOP3 (Account::FetchFlags);
		void FetchMessagesIMAP (Account::FetchFlags, const QList<QStringList>&, vmime::ref<vmime::net::store>);
		void FetchMessagesInFolder (const QStringList&, vmime::ref<vmime::net::folder>);
		void SyncIMAPFolders (vmime::ref<vmime::net::store>);
		QList<Message_ptr> FetchFullMessages (const std::vector<vmime::ref<vmime::net::message>>&);
		ProgressListener* MkPgListener (const QString&);
	public slots:
		void synchronize (Account::FetchFlags, const QList<QStringList>&);
		void fetchWholeMessage (Message_ptr);
		void fetchAttachment (Message_ptr, const QString&, const QString&);
		void sendMessage (Message_ptr);
		void timeoutDisconnect ();
	signals:
		void error (const QString&);
		void gotEntity (const LeechCraft::Entity&);
		void gotProgressListener (ProgressListener_g_ptr);
		void gotMsgHeaders (QList<Message_ptr>);
		void messageBodyFetched (Message_ptr);
		void gotUpdatedMessages (QList<Message_ptr>);
		void gotOtherMessages (QList<QByteArray>, QStringList);
		void gotFolders (QList<QStringList>);
	};
}
}
