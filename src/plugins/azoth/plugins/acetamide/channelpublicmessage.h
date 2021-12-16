/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPoint>
#include <interfaces/azoth/imessage.h>
#include "channelparticipantentry.h"

namespace LC::Azoth::Acetamide
{
	class ChannelCLEntry;

	class ChannelPublicMessage final : public QObject
									 , public IMessage
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMessage)

		QPointer<ChannelCLEntry> ParentEntry_;
		ChannelParticipantEntry_ptr ParticipantEntry_;
		QString Message_;
		QDateTime Datetime_;
		const Direction Direction_;
		const Type Type_;
		const SubType SubType_;
	public:
		ChannelPublicMessage (QString, ChannelCLEntry*);
		ChannelPublicMessage (QString, Direction,
				ChannelCLEntry*,
				Type,
				SubType,
				ChannelParticipantEntry_ptr = {});

		QObject* GetQObject () override;
		void Send () override;
		void Store () override;
		Direction GetDirection () const override;
		Type GetMessageType () const override;
		SubType GetMessageSubType () const override;
		/** Since it's outgoing message, the other part
		 * always equals to the room entry.
		 */
		QObject* OtherPart () const override;
		QObject* ParentCLEntry () const override;
		QString GetOtherVariant () const override;
		QString GetBody () const override;
		void SetBody (const QString&) override;
		QDateTime GetDateTime () const override;
		void SetDateTime (const QDateTime&) override;
	};
}
