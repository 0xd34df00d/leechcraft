/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPointer>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/irichtextmessage.h>
#include "roomparticipantentry.h"

class QXmppMessage;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class RoomCLEntry;
	class RoomParticipantEntry;

	class RoomPublicMessage : public QObject
							, public IMessage
							, public IRichTextMessage
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMessage
				LC::Azoth::IRichTextMessage)

		QPointer<RoomCLEntry> ParentEntry_;
		RoomParticipantEntry_ptr ParticipantEntry_;
		QString Message_;
		QDateTime Datetime_;
		Direction Direction_;
		QString FromJID_;
		QString FromVariant_;
		Type Type_;
		SubType SubType_;

		QString XHTML_;
	public:
		RoomPublicMessage (const QString&, RoomCLEntry*);
		RoomPublicMessage (const QString&, Direction,
				RoomCLEntry*,
				Type,
				SubType,
				RoomParticipantEntry_ptr = RoomParticipantEntry_ptr ());
		RoomPublicMessage (const QXmppMessage&, RoomCLEntry*,
				RoomParticipantEntry_ptr = RoomParticipantEntry_ptr ());

		void SetParticipantEntry (const RoomParticipantEntry_ptr&);

		QObject* GetQObject ();
		void Send ();
		void Store ();
		Direction GetDirection () const;
		Type GetMessageType () const;
		SubType GetMessageSubType () const;

		QObject* OtherPart () const;
		QObject* ParentCLEntry () const;
		QString GetOtherVariant () const;
		QString GetBody () const;
		void SetBody (const QString&);
		QDateTime GetDateTime () const;
		void SetDateTime (const QDateTime&);

		QString GetRichBody () const;
		void SetRichBody (const QString&);
	};
}
}
}
