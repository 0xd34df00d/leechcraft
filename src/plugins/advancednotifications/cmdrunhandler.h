/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_ADVANCEDNOTIFICATIONS_CMDRUNHANDLER_H
#define PLUGINS_ADVANCEDNOTIFICATIONS_CMDRUNHANDLER_H
#include <QObject>
#include <interfaces/structures.h>
#include "concretehandlerbase.h"
#include "eventdata.h"

namespace LC
{
namespace AdvancedNotifications
{
	class CmdRunHandler : public ConcreteHandlerBase
	{
		Q_OBJECT
	public:
		CmdRunHandler ();

		NotificationMethod GetHandlerMethod () const;
		void Handle (const Entity&, const NotificationRule&);
	};
}
}

#endif
