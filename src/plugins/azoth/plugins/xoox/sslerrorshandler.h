/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/icanhavesslerrors.h>

class QSslError;

template<typename>
class QList;

class QXmppClient;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class SslErrorsHandler : public QObject
	{
		Q_OBJECT

		QXmppClient * const Client_;
	public:
		SslErrorsHandler (QXmppClient*);

		void EmitAborted ();
	private slots:
		void handleSslErrors (const QList<QSslError>&);
	signals:
		void sslErrors (const QList<QSslError>&,
				const ICanHaveSslErrors::ISslErrorsReaction_ptr&);

		void aborted ();
	};
}
}
}
