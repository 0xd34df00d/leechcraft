/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "acetamide.h"
#include <QIcon>
#include <QStandardItemModel>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/azoth/iproxyobject.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "ircprotocol.h"
#include "localtypes.h"
#include "nickservidentifymanager.h"
#include "nickservidentifywidget.h"
#include "xmlsettingsmanager.h"

namespace LC::Azoth::Acetamide
{
	void Plugin::Init (ICoreProxy_ptr)
	{
#if QT_VERSION_MAJOR < 6
		qRegisterMetaTypeStreamOperators<QList<QStringList>> ("QList<QStringList>");
#endif

		SettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
					QStringLiteral ("azothacetamidesettings.xml"));

		IdentifyManager_ = std::make_shared<NickServIdentifyManager> ();

		SettingsDialog_->SetCustomWidget (QStringLiteral ("NickServIdentifyWidget"),
				IdentifyManager_->GetConfigWidget ());

		IrcProtocol_ = std::make_shared<IrcProtocol> (*IdentifyManager_, this);
	}

	void Plugin::SecondInit ()
	{
		IrcProtocol_->Prepare ();
	}

	void Plugin::Release ()
	{
		IrcProtocol_.reset ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Acetamide";
	}

	QString Plugin::GetName () const
	{
		return Lits::AzothAcetamide;
	}

	QString Plugin::GetInfo () const
	{
		return tr ("IRC protocol support.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin" };
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	QList<QObject*> Plugin::GetProtocols () const
	{
		return { IrcProtocol_.get () };
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		IrcProtocol_->SetProxyObject (qobject_cast<IProxyObject*> (proxy));
	}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_acetamide,
		LC::Azoth::Acetamide::Plugin);
