/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/ihaverecoverabletabs.h>
#include <interfaces/core/icoreproxy.h>

class QMenu;
class QAction;
class ITabWidget;

namespace LC
{
namespace TabSessManager
{
	class TabsPropsManager;

	class UncloseManager : public QObject
	{
		Q_OBJECT

		struct RemoveTabParams;

		const ICoreProxy_ptr Proxy_;

		TabsPropsManager * const TabsPropsMgr_;

		QMenu * const UncloseMenu_;
	public:
		UncloseManager (const ICoreProxy_ptr&, TabsPropsManager*, QObject* = nullptr);

		QAction* GetMenuAction () const;

		void HandleRemoveTab (QWidget*);
	private:
		void GenericRemoveTab (const RemoveTabParams&);
		void HandleRemoveRecoverableTab (QWidget*, IRecoverableTab*);
		void HandleRemoveSingleTab (QWidget*, ITabWidget*);
	};
}
}
