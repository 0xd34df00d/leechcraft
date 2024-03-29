/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "viewsettingsmanager.h"
#include <QCoreApplication>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include "viewmanager.h"

namespace LC::SB2
{
	namespace
	{
		class XmlViewSettingsManager : public Util::BaseSettingsManager
		{
			ViewManager * const ViewMgr_;
		public:
			XmlViewSettingsManager (ViewManager*);
		protected:
			Settings_ptr MakeSettings () const override;
		};

		XmlViewSettingsManager::XmlViewSettingsManager (ViewManager *view)
		: ViewMgr_ (view)
		{
			Util::BaseSettingsManager::Init ();
		}

		Settings_ptr XmlViewSettingsManager::MakeSettings () const
		{
			return ViewMgr_->GetSettings ();
		}
	}

	ViewSettingsManager::ViewSettingsManager (ViewManager *mgr)
	: QObject (mgr)
	, XSM_ (std::make_shared<XmlViewSettingsManager> (mgr))
	, XSD_ (std::make_shared<Util::XmlSettingsDialog> ())
	{
		XSD_->RegisterObject (XSM_.get (), QStringLiteral ("sb2panelsettings.xml"));
	}

	Util::XmlSettingsDialog* ViewSettingsManager::GetXSD () const
	{
		return XSD_.get ();
	}

	Util::BaseSettingsManager* ViewSettingsManager::GetXSM () const
	{
		return XSM_.get ();
	}
}
