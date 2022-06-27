/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QString>

class IPluginsManager;

namespace LC
{
namespace LMP
{
class ILocalCollection;

namespace PPL
{
	class LogHandler : public QObject
	{
		ILocalCollection * const Collection_;
	public:
		LogHandler (const QString&, ILocalCollection*, IPluginsManager*, QObject* = nullptr);
	};
}
}
}
