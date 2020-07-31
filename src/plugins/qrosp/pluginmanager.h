/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_QROSP_PLUGINMANAGER_H
#define PLUGINS_QROSP_PLUGINMANAGER_H
#include <QObject>
#include <QMap>

namespace LC
{
namespace Qrosp
{
	class PluginManager : public QObject
	{
		Q_OBJECT

		QList<QObject*> Wrappers_;
		PluginManager ();
	public:
		static PluginManager& Instance ();

		void Release ();
		QList<QObject*> GetPlugins ();
	private:
		QMap<QString, QStringList> FindPlugins ();
	};
}
}

#endif
