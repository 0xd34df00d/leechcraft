/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "browserwidgetsettingshandler.h"
#include "interfaces/poshuku/iwebview.h"
#include "browserwidget.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Poshuku
{
	BrowserWidgetSettingsHandler::BrowserWidgetSettingsHandler (BrowserWidget *widget)
	: QObject { widget }
	, View_ { widget->GetWebView () }
	{
		XmlSettingsManager::Instance ().RegisterObject ({
					"AutoLoadImages",
					"AllowJavascript",
					"AllowPlugins",
					"JavascriptCanOpenWindows",
					"JavascriptCanAccessClipboard",
					"UserStyleSheet",
					"LocalStorageDB",
					"EnableXSSAuditing",
					"EnableWebGL",
					"EnableHyperlinkAuditing",
					"EnableSmoothScrolling"
				},
				this, "viewerSettingsChanged");

		viewerSettingsChanged ();
	}

	void BrowserWidgetSettingsHandler::viewerSettingsChanged ()
	{
		auto& xsm = XmlSettingsManager::Instance ();
		auto set = [&] (IWebView::Attribute attr, const char *prop)
		{
			View_->SetAttribute (attr, xsm.property (prop).toBool ());
		};
		using enum IWebView::Attribute;
		set (AutoLoadImages, "AutoLoadImages");
		set (JavascriptEnabled, "AllowJavascript");
		set (PluginsEnabled, "AllowPlugins");
		set (JavascriptCanOpenWindows, "JavascriptCanOpenWindows");
		set (JavascriptCanAccessClipboard, "JavascriptCanAccessClipboard");
		set (LocalStorageEnabled, "LocalStorageDB");
		set (XSSAuditingEnabled, "EnableXSSAuditing");
		set (HyperlinkAuditingEnabled, "EnableHyperlinkAuditing");
		set (WebGLEnabled, "EnableWebGL");
		set (ScrollAnimatorEnabled, "EnableSmoothScrolling");
	}
}
}
