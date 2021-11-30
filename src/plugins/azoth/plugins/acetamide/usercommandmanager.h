/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QByteArray;
class QString;

namespace LC::Azoth::Acetamide
{
	class IrcServerHandler;
	class IrcParser;

	class UserCommandManager
	{
		IrcServerHandler * const ISH_;
		IrcParser * const Parser_;
	public:
		UserCommandManager (IrcServerHandler*, IrcParser*);

		QByteArray VerifyMessage (const QString&, const QString&) const;
	};
}

