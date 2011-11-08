/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_SNAILS_ACCOUNTTHREADWORKER_H
#define PLUGINS_SNAILS_ACCOUNTTHREADWORKER_H
#include <QObject>
#include <vmime/net/session.hpp>
#include <vmime/net/message.hpp>
#include "message.h"

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
	public:
		AccountThreadWorker (Account*);
	private:
		vmime::ref<vmime::net::store> MakeStore ();
		vmime::ref<vmime::net::transport> MakeTransport ();
		Message_ptr FromHeaders (const vmime::ref<vmime::net::message>&) const;
	public slots:
		void fetchNewHeaders (int);
		void rebuildSessConfig ();
	signals:
		void error (const QString&);
		void gotMsgHeaders (QList<Message_ptr>);
	};
}
}

#endif
