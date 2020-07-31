/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_JABBERSEARCHMANAGER_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_JABBERSEARCHMANAGER_H
#include <QSet>
#include <QXmppClientExtension.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class JabberSearchManager : public QXmppClientExtension
	{
		Q_OBJECT

		QSet<QString> FieldRequests_;
		QSet<QString> SearchRequests_;
	public:
		struct Item
		{
			QMap<QString, QString> Dictionary_;

			Item ();
			Item (const QString& jid,
					const QString& first, const QString& last,
					const QString& nick, const QString& email);
		};

		bool handleStanza (const QDomElement&);

		void RequestSearchFields (const QString& server);

		void SubmitSearchRequest (const QString& server, QXmppElement);
		void SubmitSearchRequest (const QString& server, const QList<QXmppElement>&);
		void SubmitSearchRequest (const QString& server, const QXmppDataForm&);
	private:
		bool CheckError (const QDomElement&);
		QList<Item> FromForm (const QDomElement&);
		QList<Item> FromStandardItems (const QDomElement&);
	signals:
		void gotSearchFields (const QString& server, const QXmppElement& containing);
		void gotItems (const QString& server, const QList<JabberSearchManager::Item>& items);
		void gotServerError (const QXmppIq& iq);
	};
}
}
}

#endif
