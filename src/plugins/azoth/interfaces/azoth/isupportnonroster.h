/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

class QString;
class QObject;

namespace LC
{
namespace Azoth
{
	class ISupportNonRoster
	{
	public:
		virtual ~ISupportNonRoster () {}

		virtual QObject* CreateNonRosterItem (const QString&) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::ISupportNonRoster,
		"org.LeechCraft.Azoth.ISupportNonRoster/1.0")
