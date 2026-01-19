/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "confmessage.h"
#include "confparticipant.h"

namespace LC::Azoth::Sarin
{
	ConfMessage::ConfMessage (const QString& body, ConfParticipant& part)
	: BaseMessage { Direction::In, Type::MUCMessage, SubType::Other, body, &part }
	, Participant_ { part }
	{
	}

	void ConfMessage::Send ()
	{
	}

	void ConfMessage::Store ()
	{
		Participant_.GetParentCLEntry ()->AppendMessage (this);
	}

	QObject* ConfMessage::OtherPart () const
	{
		return &Participant_;
	}

	QString ConfMessage::GetOtherVariant () const
	{
		return Participant_.GetEntryName ();
	}
}
