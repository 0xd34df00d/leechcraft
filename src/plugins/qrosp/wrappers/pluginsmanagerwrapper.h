/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_QROSP_WRAPPERS_PLUGINSMANAGERWRAPPER_H
#define PLUGINS_QROSP_WRAPPERS_PLUGINSMANAGERWRAPPER_H
#include <QObject>
#include <interfaces/core/ipluginsmanager.h>

namespace LC
{
namespace Qrosp
{
	class PluginsManagerWrapper : public QObject
	{
		Q_OBJECT

		IPluginsManager *Manager_;
	public:
		PluginsManagerWrapper (IPluginsManager*);
	public slots:
		QObjectList GetAllPlugins () const;
		QString GetPluginLibraryPath (const QObject*) const;
	};
}
}

#endif
