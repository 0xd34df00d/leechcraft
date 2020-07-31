/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_QROSP_WRAPPERS_SHORTCUTPROXYWRAPPER_H
#define PLUGINS_QROSP_WRAPPERS_SHORTCUTPROXYWRAPPER_H
#include <QObject>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/core/ishortcutproxy.h>

namespace LC
{
namespace Qrosp
{
	class ShortcutProxyWrapper : public QObject
	{
		Q_OBJECT

		IShortcutProxy *ShortcutProxy_;
	public:
		ShortcutProxyWrapper (IShortcutProxy*);
	public slots:
		QList<QKeySequence> GetShortcuts (QObject*, const QString&);
	};
}
}

#endif
