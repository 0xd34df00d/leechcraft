/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lhtr.h"
#include <QIcon>
#include <QtDebug>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "richeditorwidget.h"
#include "xmlsettingsmanager.h"

namespace LC::LHTR
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("lhtr");

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "lhtrsettings.xml");
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LHTR";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "LHTR";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Full-blown Blink-based HTML text editor.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	bool Plugin::SupportsEditor (ContentType type) const
	{
		switch (type)
		{
		case ContentType::HTML:
		case ContentType::PlainText:
			return true;
		}

		qWarning () << "unknown content type"
				<< static_cast<int> (type);
		return false;
	}

	QWidget* Plugin::GetTextEditor (ContentType)
	{
		return new RichEditorWidget ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}
}

LC_EXPORT_PLUGIN (leechcraft_lhtr, LC::LHTR::Plugin);
