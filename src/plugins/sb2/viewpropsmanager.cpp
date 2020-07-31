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

namespace LC
{
namespace SB2
{
	ViewPropsManager::ViewPropsManager (ViewManager *view, ViewSettingsManager *vsm, QObject *parent)
	: QObject (parent)
	, ViewMgr_ (view)
	, VSM_ (vsm)
	{
		const auto& xsm = VSM_->GetXSM ();

		xsm->RegisterObject ("CommonHoverInTimeout", this, "hoverInTimeoutChanged");
		hoverInTimeoutChanged ();

		xsm->RegisterObject ("QuarkSpacing", this, "quarkSpacingChanged");
		quarkSpacingChanged ();
	}

	void ViewPropsManager::hoverInTimeoutChanged ()
	{
		const auto timeout = VSM_->GetXSM ()->property ("CommonHoverInTimeout").toInt ();
		ViewMgr_->GetView ()->rootContext ()->setContextProperty ("commonHoverInTimeout", timeout);
	}

	void ViewPropsManager::quarkSpacingChanged ()
	{
		const auto spacing = VSM_->GetXSM ()->property ("QuarkSpacing").toInt ();
		ViewMgr_->GetView ()->rootContext ()->setContextProperty ("quarkSpacing", spacing);
	}
}
}
