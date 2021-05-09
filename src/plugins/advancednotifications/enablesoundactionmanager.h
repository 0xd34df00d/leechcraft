/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iactionsexporter.h>

class QAction;

namespace LC::AdvancedNotifications
{
	class EnableSoundActionManager : public QObject
	{
		QAction * const EnableAction_;
	public:
		explicit EnableSoundActionManager (QObject* = nullptr);

		QAction* GetAction () const;

		QList<QAction*> GetActions (ActionsEmbedPlace) const;
	};
}
