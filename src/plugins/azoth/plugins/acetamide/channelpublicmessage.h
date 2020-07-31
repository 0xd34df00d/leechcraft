/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELPUBLICMESSAGE_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELPUBLICMESSAGE_H

#include <QObject>
#include <QPoint>
#include <interfaces/azoth/imessage.h>
#include "channelparticipantentry.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{

	class ChannelCLEntry;

	class ChannelPublicMessage : public QObject
								, public IMessage
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMessage)

		QPointer<ChannelCLEntry> ParentEntry_;
		ChannelParticipantEntry_ptr ParticipantEntry_;
		QString Message_;
		QDateTime Datetime_;
		Direction Direction_;
		QString FromChID_;
		Type Type_;
		SubType SubType_;
	public:
		ChannelPublicMessage (const QString&, ChannelCLEntry*);
		ChannelPublicMessage (const QString&, Direction,
				ChannelCLEntry*,
				Type,
				SubType,
				ChannelParticipantEntry_ptr = ChannelParticipantEntry_ptr ());

		QObject* GetQObject ();
		void Send ();
		void Store ();
		Direction GetDirection () const;
		Type GetMessageType () const;
		void SetMessageType (IMessage::Type);
		SubType GetMessageSubType () const;
		void SetMessageSubType (IMessage::SubType);
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
