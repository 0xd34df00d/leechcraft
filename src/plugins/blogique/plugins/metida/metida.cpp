/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "metida.h"
#include <QIcon>
#include <util/util.h>
#include <interfaces/structures.h>
#include "ljbloggingplatform.h"
#include "localstorage.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("blogique_metida");

		XmlSettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (), "blogiquemetidasettings.xml");

		Storage_ = std::make_shared<LocalStorage> (GetUniqueID ());
		LJPlatform_ = std::make_shared<LJBloggingPlatform> (*Storage_, proxy, this);
	}

	void Plugin::SecondInit ()
	{
		LJPlatform_->Prepare ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Blogique.Metida";
	}

	void Plugin::Release ()
	{
		LJPlatform_->Release ();
	}

	QString Plugin::GetName () const
	{
		return "Blogique Metida";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("LiveJournal blogging platform support for Blogique.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon ("lcicons:/blogique/metida/resources/images/metida.svg");
		return icon;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.Plugins.Blogique.Plugins.IBlogPlatformPlugin" };
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	QList<QObject*> Plugin::GetBloggingPlatforms () const
	{
		return { LJPlatform_.get () };
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		LJPlatform_->SetPluginProxy (proxy);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_blogique_metida, LC::Blogique::Metida::Plugin);
