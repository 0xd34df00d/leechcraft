/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "xoox.h"
#include <QIcon>
#include <QTranslator>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
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
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
		connect (&Core::Instance (),
				SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)));
	}

	void Plugin::SecondInit ()
	{
		Core::Instance ().SecondInit ();
	}

	void Plugin::Release ()
	{
		Core::Instance ().Release ();
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
		static QIcon icon (":/plugins/azoth/plugins/xoox/resources/images/xoox.svg");
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

	QObject* Plugin::GetObject ()
	{
		return this;
	}

	QList<QObject*> Plugin::GetProtocols () const
	{
		return Core::Instance ().GetProtocols ();
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		Core::Instance ().SetPluginProxy (proxy);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_xoox,
		LeechCraft::Azoth::Xoox::Plugin);
