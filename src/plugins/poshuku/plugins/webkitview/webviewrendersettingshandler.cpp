/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "webviewrendersettingshandler.h"
#include <qwebview.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	WebViewRenderSettingsHandler::WebViewRenderSettingsHandler (QWebView *view)
	: QObject { view }
	, View_ { view }
	{
		XmlSettingsManager::Instance ().RegisterObject ({
					"PrimitivesAntialiasing",
					"TextAntialiasing",
					"SmoothPixmapTransform",
				},
				this, "renderSettingsChanged");
		renderSettingsChanged ();
	}

	void WebViewRenderSettingsHandler::renderSettingsChanged ()
	{
		QPainter::RenderHints hints;

		auto check = [&hints] (const char *name, QPainter::RenderHint hint)
		{
			if (XmlSettingsManager::Instance ().property (name).toBool ())
				hints |= hint;
		};
		check ("PrimitivesAntialiasing", QPainter::Antialiasing);
		check ("TextAntialiasing", QPainter::TextAntialiasing);
		check ("SmoothPixmapTransform", QPainter::SmoothPixmapTransform);

		View_->setRenderHints (hints);
	}
}
}
}
