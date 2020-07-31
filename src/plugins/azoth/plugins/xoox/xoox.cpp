/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xoox.h"
#include <QIcon>
#include <QTranslator>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/core/ipluginsmanager.h>
#include "glooxprotocol.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "rostersaver.h"
#include "capsdatabase.h"
#include "vcardstorage.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("azoth_xoox");

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "azothxooxsettings.xml");

		Core::Instance ().SetProxy (proxy);

		connect (&Core::Instance (),
				SIGNAL (gotEntity (LC::Entity)),
				this,
				SIGNAL (gotEntity (LC::Entity)));

		const auto& progRep = proxy->GetPluginsManager ()->CreateLoadProgressReporter (this);
		const auto capsDB = new CapsDatabase { progRep };

		VCardStorage_ = std::make_shared<VCardStorage> ();

		GlooxProtocol_ = std::make_shared<GlooxProtocol> (capsDB, VCardStorage_.get ());
	}

	void Plugin::SecondInit ()
	{
		GlooxProtocol_->SetProxyObject (PluginProxy_);
		GlooxProtocol_->Prepare ();

		new RosterSaver
		{
			GlooxProtocol_.get (),
			PluginProxy_,
			GlooxProtocol_.get ()
		};
	}

	void Plugin::Release ()
	{
		GlooxProtocol_.reset ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Xoox";
	}

	QString Plugin::GetName () const
	{
		return "Azoth Xoox";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("XMPP (Jabber) protocol module using the QXmpp library.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon ("lcicons:/plugins/azoth/plugins/xoox/resources/images/xoox.svg");
		return icon;
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin";
		return classes;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	QList<QObject*> Plugin::GetProtocols () const
	{
		return { GlooxProtocol_.get () };
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		PluginProxy_ = qobject_cast<IProxyObject*> (proxy);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_xoox,
		LC::Azoth::Xoox::Plugin);
