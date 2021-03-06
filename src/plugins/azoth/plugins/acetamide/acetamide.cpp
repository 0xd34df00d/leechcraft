/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "acetamide.h"
#include <ctime>
#include <QIcon>
#include <QStandardItemModel>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "nickservidentifywidget.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Translator_.reset (Util::InstallTranslator ("azoth_acetamide"));

		qsrand (time (NULL));

		qRegisterMetaTypeStreamOperators<QList<QStringList>> ("QList<QStringList>");

		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
					"azothacetamidesettings.xml");

		Core::Instance ().SetProxy (proxy);

		SettingsDialog_->SetCustomWidget ("NickServIdentifyWidget",
				Core::Instance ().GetNickServIdentifyWidget ());

		connect (&Core::Instance (),
				SIGNAL (gotEntity (const LC::Entity&)),
				this,
				SIGNAL (gotEntity (const LC::Entity&)));
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
		return "org.LeechCraft.Azoth.Acetamide";
	}

	QString Plugin::GetName () const
	{
		return "Azoth Acetamide";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("IRC protocol support.");
	}

	QIcon Plugin::GetIcon () const
	{
		return Core::Instance ().GetProxy ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin";
		return classes;
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	QList<QObject*> Plugin::GetProtocols () const
	{
		return Core::Instance ().GetProtocols ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		Core::Instance ().SetPluginProxy (proxy);
	}
}
}
}

QDataStream& operator<< (QDataStream& out, const QList<QStringList>& list)
{
	for (const auto& subList : list)
		out << subList;

	return out;
}

QDataStream& operator>> (QDataStream& in, QList<QStringList>& list)
{
	QStringList subList;
	while (!in.atEnd ())
	{
		in >> subList;
		list << subList;
		subList.clear ();
	}

	return in;
}

LC_EXPORT_PLUGIN (leechcraft_azoth_acetamide,
		LC::Azoth::Acetamide::Plugin);
