/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xmppelementdescription.h"
#include <QXmppElement.h>
#include <util/sll/qtutil.h>

namespace LC::Azoth::Xoox
{
	QXmppElement ToElement (const XmppElementDescription& descr)
	{
		QXmppElement elem;
		elem.setTagName (descr.TagName_);
		if (!descr.Value_.isEmpty ())
			elem.setValue (descr.Value_);

		for (const auto& [attr, value] : Util::Stlize (descr.Attributes_))
			elem.setAttribute (attr, value);

		for (const auto& child : descr.Children_)
			elem.appendChild (ToElement (child));

		return elem;
	}
}
