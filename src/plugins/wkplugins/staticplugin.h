/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QWebKitPlatformPlugin>

namespace LC
{
namespace WKPlugins
{
	class StaticPlugin : public QObject
					   , public QWebKitPlatformPlugin
	{
		Q_OBJECT
		Q_INTERFACES (QWebKitPlatformPlugin)

		Q_PLUGIN_METADATA (IID "org.qtwebkit.QtWebKit.QtWebKitPlugins")

		static QWebKitPlatformPlugin *Impl_;
	public:
		static void SetImpl (QWebKitPlatformPlugin*);

		bool supportsExtension (Extension) const override;
		QObject* createExtension (Extension) const override;
	};
}
}
