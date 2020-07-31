/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ooronee.h"
#include <QIcon>
#include <QtQuick>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "droparea.h"
#include "quarkproxy.h"

namespace LC
{
namespace Ooronee
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("ooronee");

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "ooroneesettings.xml");

		qmlRegisterType<DropArea> ("org.LC.Ooronee", 1, 0, "DropArea");

		Quark_ = std::make_shared<QuarkComponent> ("ooronee", "OoroneeQuark.qml");
		Quark_->DynamicProps_.append ({ "Ooronee_Proxy", new QuarkProxy { proxy } });
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Ooronee";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Ooronee";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides a quark for handling image and text droppend onto it via other data filter plugins.");
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
		return { Quark_ };
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_ooronee, LC::Ooronee::Plugin);
