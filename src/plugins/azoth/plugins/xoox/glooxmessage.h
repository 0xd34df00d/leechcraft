/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXMESSAGE_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXMESSAGE_H
#include <QObject>
#include <interfaces/imessage.h>

namespace gloox
{
	class MessageSession;
	class Message;
}

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
namespace Xoox
{
	class GlooxCLEntry;

	class GlooxMessage : public QObject
						, public IMessage
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Plugins::Azoth::Plugins::IMessage)

		MessageType Type_;
		Direction Direction_;
		QObject *Entry_;
		QString Body_;
		QString Variant_;
		gloox::MessageSession *Session_;
		QDateTime DateTime_;
	public:
		GlooxMessage (IMessage::MessageType type,
				IMessage::Direction direction,
				QObject *entry,
				gloox::MessageSession *session);
		GlooxMessage (const gloox::Message& msg,
				QObject *entry,
				gloox::MessageSession *session);

		QObject* GetObject ();
		void Send ();
		Direction GetDirection () const;
		MessageType GetMessageType () const;
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
}
}

#endif
