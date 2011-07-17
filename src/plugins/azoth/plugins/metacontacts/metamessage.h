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

#ifndef PLUGINS_AZOTH_PLUGINS_METACONTACTS_METAMESSAGE_H
#define PLUGINS_AZOTH_PLUGINS_METACONTACTS_METAMESSAGE_H
#include <QObject>
#include <interfaces/imessage.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Metacontacts
{
	class MetaEntry;

	class MetaMessage : public QObject
					  , public IMessage
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IMessage);
		
		MetaEntry *Entry_;
		QObject *MessageObj_;
		IMessage *Message_;
	public:
		MetaMessage (QObject*, MetaEntry*);
		
		QObject* GetObject ();
		void Send ();
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
