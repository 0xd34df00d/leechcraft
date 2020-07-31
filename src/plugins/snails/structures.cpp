/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "structures.h"
#include <QtDebug>
#include <util/sll/unreachable.h>
#include <interfaces/itexteditor.h>

namespace LC
{
namespace Snails
{
	QString GetBasename (MsgType type)
	{
		switch (type)
		{
		case MsgType::New:
			return "new";
		case MsgType::Reply:
			return "reply";
		case MsgType::Forward:
			return "forward";
		}

		Util::Unreachable ();
	}

	QString GetExtension (ContentType type)
	{
		switch (type)
		{
		case ContentType::PlainText:
			return "txt";
		case ContentType::HTML:
			return "html";
		}

		Util::Unreachable ();
	}
}
}
