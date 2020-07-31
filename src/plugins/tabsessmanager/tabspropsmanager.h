/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QVariant>
#include <util/sll/util.h>
#include <interfaces/core/ihookproxy.h>

class QWidget;

namespace LC
{
namespace TabSessManager
{
	class TabsPropsManager
	{
	public:
		using TabsProps_t = QList<QPair<QByteArray, QVariant>>;
	private:
		QList<TabsProps_t> TabsPropsQueue_;
		QList<int> PreferredWindowsQueue_;
	public:
		Q_REQUIRED_RESULT Util::DefaultScopeGuard AppendProps (const TabsProps_t&);
		Q_REQUIRED_RESULT Util::DefaultScopeGuard AppendWindow (int);

		void HandlePreferredWindowIndex (const IHookProxy_ptr&, const QWidget*);
		void HandleTabAdding (QWidget*);
	};
}
}
