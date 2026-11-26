/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "plaintext.h"
#include <QTextDocumentFragment>

namespace LC::AdvancedNotifications
{
	QString ToPlainTextLine (const Util::RichTextString& text)
	{
		auto plain = QTextDocumentFragment::fromHtml (text.ToHtml ()).toPlainText ();
		plain.remove (QChar::ObjectReplacementCharacter);
		return plain.simplified ();
	}
}
