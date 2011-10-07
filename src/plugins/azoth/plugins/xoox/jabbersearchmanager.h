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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_JABBERSEARCHMANAGER_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_JABBERSEARCHMANAGER_H
#include <QSet>
#include <QXmppClientExtension.h>

namespace LeechCraft
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
