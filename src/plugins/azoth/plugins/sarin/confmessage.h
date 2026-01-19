/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "basemessage.h"

namespace LC::Azoth::Sarin
{
	class ConfEntry;
	class ConfParticipant;

	class ConfMessage
		: public BaseMessage
	{
		ConfParticipant& Participant_;
	public:
		explicit ConfMessage (const QString& body, ConfParticipant& part);

		void Send () override;
		void Store () override;
		QObject* OtherPart () const override;
		QString GetOtherVariant () const override;
	};
}
