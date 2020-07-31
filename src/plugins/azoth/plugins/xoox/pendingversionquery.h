/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/ihavequeriableversion.h>

class QXmppVersionManager;
class QXmppVersionIq;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class PendingVersionQuery : public QObject
							  , public IPendingVersionQuery
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IPendingVersionQuery)

		const QString Jid_;
	public:
		PendingVersionQuery (QXmppVersionManager *vm, const QString& reqJid, QObject *parent);
	private slots:
		void handleVersionReceived (const QXmppVersionIq&);
	signals:
		void versionReceived ();
	};
}
}
}
