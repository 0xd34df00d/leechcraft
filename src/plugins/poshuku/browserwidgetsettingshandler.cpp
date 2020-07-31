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
		XmlSettingsManager::Instance ()->RegisterObject ({
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
		auto xsm = XmlSettingsManager::Instance ();

		View_->SetAttribute (IWebView::Attribute::AutoLoadImages,
				xsm->property ("AutoLoadImages").toBool ());
		View_->SetAttribute (IWebView::Attribute::JavascriptEnabled,
				xsm->property ("AllowJavascript").toBool ());
		View_->SetAttribute (IWebView::Attribute::PluginsEnabled,
				xsm->property ("AllowPlugins").toBool ());
		View_->SetAttribute (IWebView::Attribute::JavascriptCanOpenWindows,
				xsm->property ("JavascriptCanOpenWindows").toBool ());
		View_->SetAttribute (IWebView::Attribute::JavascriptCanAccessClipboard,
				xsm->property ("JavascriptCanAccessClipboard").toBool ());
		View_->SetAttribute (IWebView::Attribute::LocalStorageEnabled,
				xsm->property ("LocalStorageDB").toBool ());
		View_->SetAttribute (IWebView::Attribute::XSSAuditingEnabled,
				xsm->property ("EnableXSSAuditing").toBool ());
		View_->SetAttribute (IWebView::Attribute::HyperlinkAuditingEnabled,
				xsm->property ("EnableHyperlinkAuditing").toBool ());
		View_->SetAttribute (IWebView::Attribute::WebGLEnabled,
				xsm->property ("EnableWebGL").toBool ());
		View_->SetAttribute (IWebView::Attribute::ScrollAnimatorEnabled,
				xsm->property ("EnableSmoothScrolling").toBool ());
	}
}
}
