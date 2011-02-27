/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELPUBLICMESSAGE_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELPUBLICMESSAGE_H

#include <QObject>
#include <QPoint>
#include <interfaces/imessage.h>
#include "channelparticipantentry.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class ChannelCLEntry;
	class ChannelParticipantEntry;
	
	class ChannelPublicMessage : public QObject
								, public IMessage
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IMessage);
		
		QPointer<ChannelCLEntry> ParentEntry_;
		ChannelParticipantEntry_ptr ParticipantEntry_;
		QString Message_;
		QDateTime Datetime_;
		Direction Direction_;
		QString FromChID_;
		QString FromVariant_;
		MessageType Type_;
		MessageSubType SubType_;
	public:
		ChannelPublicMessage (const QString&, ChannelCLEntry*);
		ChannelPublicMessage (const QString&, Direction,
				ChannelCLEntry*,
				MessageType,
				MessageSubType,
				ChannelParticipantEntry_ptr = ChannelParticipantEntry_ptr ());
		
		QObject* GetObject ();
		void Send ();
		Direction GetDirection () const;
		MessageType GetMessageType () const;
		MessageSubType GetMessageSubType () const;

		/** Since it's outgoing message, the other part
		 * always equals to the room entry.
		 */
		QObject* OtherPart () const;
		QObject* ParentCLEntry () const;
		QString GetOtherVariant () const;
		QString GetBody () const;
		void SetBody (const QString&);
		QDateTime GetDateTime () const;
		void SetDateTime (const QDateTime&);
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELPUBLICMESSAGE_H
