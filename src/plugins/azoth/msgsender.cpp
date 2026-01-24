/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "msgsender.h"
#include "interfaces/azoth/iclentry.h"
#include "hooksinstance.h"

namespace LC
{
namespace Azoth
{
	MsgSender::MsgSender (ICLEntry *e, IMessage::Type type, QString text, QString variantStr, QString richText)
	{
		deleteLater ();

		std::optional<QString> variant;
		if (!variantStr.isEmpty ())
			variant = variantStr;
		OutgoingMessage message { .Variant_ = variant, .Body_ = text, .RichTextBody_ = richText };

		bool cancel = false;
		emit HooksInstance::Instance ().messageWillBeCreated (cancel, *e, message);
		if (!cancel)
			e->SendMessage (message);
	}
}
}
