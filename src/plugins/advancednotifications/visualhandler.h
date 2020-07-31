/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/structures.h>
#include "concretehandlerbase.h"
#include "eventdata.h"

namespace LC
{
namespace AdvancedNotifications
{
	class VisualHandler : public ConcreteHandlerBase
	{
		Q_OBJECT

		QSet<QString> ActiveEvents_;
	public:
		NotificationMethod GetHandlerMethod () const;
		void Handle (const Entity&, const NotificationRule&);
	private slots:
		void handleProbeDestroyed ();
	};
}
}
