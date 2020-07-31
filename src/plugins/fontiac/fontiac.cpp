/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fontiac.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include "xmlsettingsmanager.h"
#include "substsmanager.h"

namespace LC
{
namespace Fontiac
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("fontiac");

		const auto substsManager = new SubstsManager;

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "fontiacsettings.xml");
		XSD_->SetDataSource ("SubstitutionsView", substsManager->GetModel ());
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Fontiac";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Fontiac";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Font substitutions configuration module.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_fontiac, LC::Fontiac::Plugin);
