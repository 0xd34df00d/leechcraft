/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "viewpropsmanager.h"
#include <QQmlContext>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include "viewsettingsmanager.h"
#include "viewmanager.h"
#include "sbview.h"

namespace LC::SB2
{
	ViewPropsManager::ViewPropsManager (ViewManager *view, ViewSettingsManager *vsm, QObject *parent)
	: QObject (parent)
	{
		const auto& xsm = vsm->GetXSM ();
		const auto ctx = view->GetView ()->rootContext ();
		xsm->RegisterObject ("CommonHoverInTimeout", this,
				[ctx] (const QVariant& prop)
				{
					ctx->setContextProperty (QStringLiteral ("commonHoverInTimeout"), prop.toInt ());
				});

		xsm->RegisterObject ("QuarkSpacing", this,
				[ctx] (const QVariant& prop)
				{
					ctx->setContextProperty (QStringLiteral ("quarkSpacing"), prop.toInt ());
				});
	}
}
