/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <unordered_set>
#include <QObject>

class QTcpSocket;
class QSettings;

class ISyncProxy;

namespace Laretz
{
	struct ParseResult;

	class Item;
	class Operation;
}

namespace LC
{
namespace Syncer
{
	class SingleSyncable : public QObject
	{
		Q_OBJECT

		const QByteArray ID_;
		ISyncProxy * const Proxy_;

		QTcpSocket * const Socket_;

		enum class State
		{
			Idle,
			ListRequested,
			FetchRequested,
			Sent,
			RootCreateRequested
		} State_ = State::Idle;

		bool IsFirstSync_ = true;

		std::unordered_set<std::string> RemoteIDs_;
	public:
		SingleSyncable (const QByteArray& id, ISyncProxy *proxy, QObject* = 0);
	private:
		std::shared_ptr<QSettings> GetSettings ();

		void HandleList (const Laretz::ParseResult&);
		void HandleFetch (const Laretz::ParseResult&);
		void HandleSendResult (const Laretz::ParseResult&);
		void HandleRootCreated (const Laretz::ParseResult&);
		void CreateRoot ();
	private slots:
		void handleSocketRead ();

		void startSync ();
		void handleSocketConnected ();
	};
}
}
