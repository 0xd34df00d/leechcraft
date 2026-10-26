/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QHash>
#include <QObject>
#include <QVariant>
#include <util/sll/util.h>
#include <interfaces/core/ihookproxy.h>

class QWidget;

namespace LC::TabSessManager
{
	class TabsPropsManager : public QObject
	{
	public:
		using TabsProps_t = QList<QPair<QByteArray, QVariant>>;
	private:
		QList<TabsProps_t> TabsPropsQueue_;
		QList<int> PreferredWindowsQueue_;
		QList<int> PositionsQueue_;
		QHash<QWidget*, int> RestoredPositions_;
	public:
		Q_REQUIRED_RESULT Util::DefaultScopeGuard AppendProps (const TabsProps_t&);
		Q_REQUIRED_RESULT Util::DefaultScopeGuard AppendWindow (int);
		Q_REQUIRED_RESULT Util::DefaultScopeGuard AppendPosition (int);

		void HandlePreferredWindowIndex (const IHookProxy_ptr&, const QWidget*);
		void HandleTabAdding (QWidget*);

		std::optional<int> TakeRestoredPosition (QWidget*);
	};
}
