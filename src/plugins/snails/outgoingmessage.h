/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include "address.h"
#include "attdescr.h"

namespace LC::Snails
{
	struct OutgoingMessage
	{
		Address From_;
		Addresses_t To_;
		Addresses_t Cc_;
		Addresses_t Bcc_;

		QList<QByteArray> InReplyTo_;
		QList<QByteArray> References_;

		QString Subject_;

		QString Body_;
		QString HTMLBody_;

		QList<AttDescr> Attachments_;
	};

	using OutgoingMessage_ptr = std::shared_ptr<OutgoingMessage>;
}
