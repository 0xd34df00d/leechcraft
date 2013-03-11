/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "lhtr.h"
#include <QIcon>
#include <QtDebug>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "richeditorwidget.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace LHTR
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Util::InstallTranslator ("lhtr");

		XSD_.reset (new Util::XmlSettingsDialog);
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
		return tr ("LeechCraft HTML Text editoR.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	bool Plugin::SupportsEditor (ContentType type) const
	{
		switch (type)
		{
		case ContentType::HTML:
		case ContentType::PlainText:
			return true;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown content type"
				<< static_cast<int> (type);
		return false;
	}

	QWidget* Plugin::GetTextEditor (ContentType)
	{
		return new RichEditorWidget (Proxy_);
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_lhtr, LeechCraft::LHTR::Plugin);
