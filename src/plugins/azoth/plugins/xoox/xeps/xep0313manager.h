/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QXmppClientExtension.h>
#include <interfaces/azoth/ihaveserverhistory.h>

class QXmppMessage;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class ClientConnection;
	class Xep0313PrefIq;

	class Xep0313Manager : public QXmppClientExtension
	{
		Q_OBJECT

		ClientConnection& Conn_;

		QMap<QString, SrvHistMessages_t> Messages_;
		QMap<QString, QString> QueryId2Jid_;

		int NextQueryNumber_ = 0;
	public:
		static bool Supports0313 (const QStringList& features);
		static QString GetNsUri ();

		explicit Xep0313Manager (ClientConnection&);

		QStringList discoveryFeatures () const override;
		bool handleStanza (const QDomElement&) override;

		void RequestPrefs ();
		void SetPrefs (const Xep0313PrefIq&);

		void RequestHistory (const QString& jid, QString baseId, int count);
	private:
		void HandleMessage (const QXmppElement&);
		void HandleHistoryQueryFinished (const QDomElement&);

		void HandlePrefs (const QDomElement&);
	signals:
		void gotPrefs (const Xep0313PrefIq&);

		void serverHistoryFetched (const QString&,
				const QString&, const SrvHistMessages_t&);
	};
}
}
}
