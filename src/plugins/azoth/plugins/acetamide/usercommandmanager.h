/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QHash>

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	class IrcServerHandler;
	class IrcParser;

	class UserCommandManager : public QObject
	{
		IrcServerHandler *ISH_;
		QHash<QString, std::function<void (QStringList)>> Command2Action_;
	public:
		UserCommandManager (IrcServerHandler*, IrcParser *parser);
		QString VerifyMessage (const QString&, const QString&);
	};
}
}
}

