/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "staticplugin.h"
#include <QtPlugin>
#include <QtDebug>

namespace LC
{
namespace WKPlugins
{
	QWebKitPlatformPlugin* StaticPlugin::Impl_ = nullptr;

	void StaticPlugin::SetImpl (QWebKitPlatformPlugin *impl)
	{
		Impl_ = impl;
	}

	bool StaticPlugin::supportsExtension (Extension ext) const
	{
		return Impl_ ? Impl_->supportsExtension (ext) : false;
	}

	QObject* StaticPlugin::createExtension (Extension ext) const
	{
		return Impl_ ? Impl_->createExtension (ext) : nullptr;
	}
}
}

Q_IMPORT_PLUGIN (StaticPlugin)
