/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xep0334utils.h"
#include <QXmppMessage.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
namespace Xep0334
{
	const QString NsMessageHints { "urn:xmpp:hints" };

	void SetHint (QXmppMessage& msg, MessageHint hint)
	{
		QXmppElement elem;
		switch (hint)
		{
		case MessageHint::NoPermStorage:
			elem.setTagName ("no-permanent-storage");
			break;
		case MessageHint::NoStorage:
			elem.setTagName ("no-storage");
			break;
		case MessageHint::NoCopies:
			elem.setTagName ("no-copies");
			break;
		}
		elem.setAttribute ("xmlns", NsMessageHints);

		auto exts = msg.extensions ();
		exts.append (elem);
		msg.setExtensions (exts);
	}
}
}
}
}
