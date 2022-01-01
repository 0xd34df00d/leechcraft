/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QHash>
#include <qwebpluginfactory.h>
#include <interfaces/core/ipluginsmanager.h>
#include "interfaces/poshuku/iwebplugin.h"

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	class WebPluginFactory : public QWebPluginFactory
	{
		Q_OBJECT

		IPluginsManager * const PM_;

		QList<IWebPlugin*> Plugins_;
		using MIME2Plugin_t = QHash<QString, QList<IWebPlugin*>>;
		MIME2Plugin_t MIME2Plugin_;
	public:
		WebPluginFactory (IPluginsManager*, QObject* = nullptr);

		QObject* create (const QString&, const QUrl&,
				const QStringList&, const QStringList&) const;
		QList<Plugin> plugins () const;
		void refreshPlugins ();
	private:
		void Reload ();
	};
}
}
}
