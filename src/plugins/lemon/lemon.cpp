/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lemon.h"
#include <QIcon>
#include <QAbstractItemModel>
#include <util/util.h>
#include <util/sys/paths.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "trafficmanager.h"
#include "xmlsettingsmanager.h"
#include "quarkproxy.h"

namespace LC
{
namespace Lemon
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("lemon");

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "lemonsettings.xml");

		Core::Instance ().SetProxy (proxy);

		TrafficMgr_ = new TrafficManager;

		PanelComponent_ = std::make_shared<QuarkComponent> ("lemon", "LemonQuark.qml");
		PanelComponent_->DynamicProps_.append ({ "Lemon_infoModel", TrafficMgr_->GetModel () });
		PanelComponent_->DynamicProps_.append ({ "Lemon_proxy", new QuarkProxy { TrafficMgr_ } });
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Lemon";
	}

	void Plugin::Release ()
	{
		Core::Instance ().Release ();
	}

	QString Plugin::GetName () const
	{
		return "Lemon";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Global network status monitor.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	QuarkComponents_t Plugin::GetComponents () const
	{
		return { PanelComponent_ };
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_lemon, LC::Lemon::Plugin);
