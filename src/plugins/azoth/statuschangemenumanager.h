/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>

class QMenu;
class QAction;

namespace LC
{
namespace Azoth
{
	class StatusChangeMenuManager : public QObject
	{
		Q_OBJECT

		struct MenuInfo
		{
			QObject *Obj_;
			const char *Slot_;
			QAction *CustomAction_;
		};
		QHash<QObject*, MenuInfo> Infos_;
	public:
		StatusChangeMenuManager (QObject* = 0);

		QMenu* CreateMenu (QObject *obj, const char *slot, QWidget *parent = 0, bool autoupdate = true);
		void UpdateCustomStatuses (QMenu*);
	private slots:
		void updateCustomStatuses ();
		void handleMenuDestroyed ();
	};
}
}
