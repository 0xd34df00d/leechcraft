/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "knowhow.h"
#include <QIcon>
#include <QTimer>
#include <QDomDocument>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "tipdialog.h"

namespace LC
{
namespace KnowHow
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		SettingsDialog_.reset (new Util::XmlSettingsDialog ());
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"knowhowsettings.xml");
	}

	void Plugin::SecondInit ()
	{
		if (XmlSettingsManager::Instance ()
				.property ("ShowTips").toBool ())
			QTimer::singleShot (10000,
					this,
					SLOT (showTip ()));
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.KnowHow";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "KnowHow";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("A simple plugin providing various tips of the day regarding LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	void Plugin::showTip ()
	{
		new TipDialog (Proxy_);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_knowhow, LC::KnowHow::Plugin);
