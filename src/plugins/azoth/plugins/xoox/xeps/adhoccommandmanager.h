/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSet>
#include <QXmppClientExtension.h>
#include <QXmppDataForm.h>
#include "adhoccommand.h"

class QXmppDiscoveryIq;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class ClientConnection;

	class AdHocCommandManager : public QXmppClientExtension
	{
		Q_OBJECT

		ClientConnection *ClientConn_;
		QSet<QString> PendingCommands_;
	public:
		static QString GetAdHocFeature ();

		explicit AdHocCommandManager (ClientConnection*);

		QString QueryCommands (const QString&);
		QString ExecuteCommand (const QString&, const AdHocCommand&);
		QString ProceedExecuting (const QString&, const AdHocResult&, const QString&);

		QStringList discoveryFeatures () const override;
		bool handleStanza (const QDomElement&) override;
	private:
		void RegisterErrorHandler (const QString&);
		void HandleError (const QXmppIq&);
	private slots:
		void handleItemsReceived (const QXmppDiscoveryIq&);
	signals:
		void gotCommands (const QString&, const QList<AdHocCommand>&);
		void gotResult (const QString&, const AdHocResult&);
		void gotError (const QString&, const QString&);
	};
}
}
}
