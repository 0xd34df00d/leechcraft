/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/azoth/iadvancedmessage.h>
#include "basemessage.h"

namespace LC::Azoth::Sarin
{
	class ToxContact;

	class ChatMessage
		: public BaseMessage
		, public IAdvancedMessage
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IAdvancedMessage)

		ToxContact * const Contact_;
		bool IsDelivered_ = false;
	public:
		ChatMessage (const QString&, Direction, ToxContact*);

		void Store () override;

		QObject* OtherPart () const override;
		QString GetOtherVariant () const override;

		bool IsDelivered () const override;
		void SetDelivered ();
	signals:
		void messageDelivered () override;
	};
}
