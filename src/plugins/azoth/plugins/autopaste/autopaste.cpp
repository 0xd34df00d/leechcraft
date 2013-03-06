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

#include "autopaste.h"
#include <QIcon>
#include <QMessageBox>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/azoth/iclentry.h>
#include "xmlsettingsmanager.h"
#include "codepadservice.h"
#include "pastedialog.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Autopaste
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("azoth_autopaste");

		Proxy_ = proxy;

		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog);
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothautopastesettings.xml");
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Autopaste";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Autopaste";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Detects long messages and suggests pasting them to a pastebin.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon (":/plugins/azoth/plugins/autopaste/resources/images/autopaste.svg");
		return icon;
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	void Plugin::hookMessageWillCreated (LeechCraft::IHookProxy_ptr proxy,
			QObject*, QObject *entry, int, QString)
	{
		ICLEntry *other = qobject_cast<ICLEntry*> (entry);
		if (!other)
		{
			qWarning () << Q_FUNC_INFO
				<< "unable to cast"
				<< entry
				<< "to ICLEntry";
			return;
		}

		QString text = proxy->GetValue ("text").toString ();

		const int maxLines = XmlSettingsManager::Instance ()
				.property ("LineCount").toInt ();
		if (text.split ('\n').size () < maxLines)
			return;

		QByteArray propName;
		switch (other->GetEntryType ())
		{
		case ICLEntry::ETChat:
			propName = "EnableForNormalChats";
			break;
		case ICLEntry::ETMUC:
			propName = "EnableForMUCChats";
			break;
		case ICLEntry::ETPrivateChat:
			propName = "EnableForPrivateChats";
			break;
		default:
			return;
		}

		if (!XmlSettingsManager::Instance ().property (propName).toBool ())
			return;

		PasteDialog dia;
		dia.exec ();
		auto choice = dia.GetChoice ();
		switch (choice)
		{
		case PasteDialog::Cancel:
			proxy->CancelDefault ();
		case PasteDialog::No:
			return;
		case PasteDialog::Yes:
		{
			auto service = dia.GetCreator () (entry);
			service->Paste ({ Proxy_->GetNetworkAccessManager (), text, dia.GetHighlight () });
			proxy->CancelDefault ();
		}
		}
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_autopaste, LeechCraft::Azoth::Autopaste::Plugin);
