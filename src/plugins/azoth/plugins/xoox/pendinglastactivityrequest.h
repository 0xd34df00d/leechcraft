/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/isupportlastactivity.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class PendingLastActivityRequest : public QObject
									 , public IPendingLastActivityRequest
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IPendingLastActivityRequest)

		const QString Jid_;

		int Time_ = 0;
	public:
		PendingLastActivityRequest (const QString& jid, QObject* = nullptr);

		int GetTime () const;

		Context GetContext () const;
	public slots:
		void handleGotLastActivity (const QString& jid, int);
	signals:
		void gotLastActivity ();
	};
}
}
}
