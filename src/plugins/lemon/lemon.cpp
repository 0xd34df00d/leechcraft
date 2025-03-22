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
#include <util/sys/paths.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "trafficmanager.h"
#include "xmlsettingsmanager.h"
#include "quarkproxy.h"

#ifdef Q_OS_LINUX
#include "linuxplatformbackend.h"
#endif

namespace LC::Lemon
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "lemonsettings.xml");

		try
		{
#ifdef Q_OS_LINUX
			Backend_ = std::make_shared<LinuxPlatformBackend> ();
#endif
		}
		catch (const std::exception& e)
		{
			qCritical () << "unable to initialize platform backend:"
					<< e.what ();
		}

		TrafficMgr_ = new TrafficManager { Backend_ };

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
		Backend_.reset ();
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

LC_EXPORT_PLUGIN (leechcraft_lemon, LC::Lemon::Plugin);
