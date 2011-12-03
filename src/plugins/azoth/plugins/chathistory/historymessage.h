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

#ifndef PLUGINS_AZOTH_PLUGINS_CHATHISTORY_HISTORYMESSAGE_H
#define PLUGINS_AZOTH_PLUGINS_CHATHISTORY_HISTORYMESSAGE_H
#include <QObject>
#include <interfaces/imessage.h>

namespace LeechCraft
{
namespace Azoth
{
namespace ChatHistory
{
	class HistoryMessage : public QObject
						 , public IMessage
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IMessage);

		Direction Direction_;
		QObject *OtherPart_;
		QString Variant_;
		QString Body_;
		QDateTime DateTime_;
	public:
		HistoryMessage (Direction dir,
				QObject *other,
				const QString& variant,
				const QString& body,
				const QDateTime& datetime);

		QObject* GetObject ();
		void Send ();
		void Store ();
		Direction GetDirection () const;
		MessageType GetMessageType () const;
		MessageSubType GetMessageSubType () const;
		QObject* OtherPart () const;
		QString GetOtherVariant () const;
		QString GetBody () const;
		void SetBody (const QString&);
		QDateTime GetDateTime () const;
		void SetDateTime (const QDateTime&);
	};
}
}
}

#endif
