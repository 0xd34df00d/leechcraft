/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "flashonclickplugin.h"
#include <algorithm>
#include <QDebug>
#include <interfaces/poshuku/iflashoverrider.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "xmlsettingsmanager.h"
#include "flashplaceholder.h"
#include "flashonclickwhitelist.h"

namespace LC
{
namespace Poshuku
{
namespace FOC
{
	FlashOnClickPlugin::FlashOnClickPlugin (const ICoreProxy_ptr& proxy,
			FlashOnClickWhitelist *wl, QObject *parent)
	: QObject { parent }
	, Proxy_ { proxy }
	, WL_ { wl }
	{
	}

	boost::optional<QWebPluginFactory::Plugin> FlashOnClickPlugin::Plugin (bool isq) const
	{
		if (isq)
			return {};

		QWebPluginFactory::Plugin result;
		result.name = "FlashOnClickPlugin";
		QWebPluginFactory::MimeType mime;
		mime.fileExtensions << "swf";
		mime.name = "application/x-shockwave-flash";
		result.mimeTypes << mime;
		return result;
	}

	QWidget* FlashOnClickPlugin::Create (const QString&,
			const QUrl& url,
			const QStringList&,
			const QStringList&)
	{
		if (!XmlSettingsManager::Instance ().property ("EnableFlashOnClick").toBool ())
			return nullptr;

		if (WL_->Matches (url.toString ()))
			return nullptr;

		const auto& overs = Proxy_->GetPluginsManager ()->GetAllCastableTo<IFlashOverrider*> ();
		if (std::any_of (overs.begin (), overs.end (),
					[&url] (IFlashOverrider *plugin) { return plugin->WouldOverrideFlash (url); }))
			return nullptr;

		return new FlashPlaceHolder { url, WL_ };
	}
}
}
}
