/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "fatape.h"
#include <QDesktopServices>
#include <QDir>
#include <QIcon>
#include <QNetworkRequest>
#include <QProcess>
#include <QStringList>
#include <QTranslator>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "userscriptsmanagerwidget.h"
#include "userscriptinstallerdialog.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace FatApe
{
	template<typename Iter, typename Pred, typename Func>
	void apply_if (Iter first, Iter last, Pred pred, Func func)
	{
		for (; first != last; ++first)
			if (pred (*first))
				func (*first);
	}

	void WrapText (QString& text, int width = 80)
	{
		int curWidth = width;

		while (curWidth < text.length ())
		{
			int spacePos = text.lastIndexOf (' ', curWidth);

			if (spacePos == -1)
				spacePos = text.indexOf (' ', curWidth);
			if (spacePos != -1)
			{
				text [spacePos] = '\n';
				curWidth = spacePos + width + 1;
			}
		}
	}

	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		CoreProxy_ = proxy;
		Translator_.reset (Util::InstallTranslator ("poshuku_fatape"));

		QDir scriptsDir (Util::CreateIfNotExists ("data/poshuku/fatape/scripts"));

		if (!scriptsDir.exists ())
			return;

		QStringList filter ("*.user.js");

		Q_FOREACH (const QString& script, scriptsDir.entryList (filter, QDir::Files))
			UserScripts_.append (UserScript (scriptsDir.absoluteFilePath (script)));

		Model_.reset (new QStandardItemModel);
		Model_->setHorizontalHeaderLabels (QStringList (tr ("Name"))
				<< tr ("Description"));
		Q_FOREACH (const UserScript& script, UserScripts_)
			AddScriptToManager (script);

		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"poshukufatapesettings.xml");
		SettingsDialog_->SetCustomWidget ("UserScriptsManagerWidget",
				new UserScriptsManagerWidget (Model_.get (), this));
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.FatApe";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku FatApe";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("GreaseMonkey support layer for the Poshuku browser.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/plugins/poshuku/plugins/fatape/resources/images/fatape.svg");
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	void Plugin::hookInitialLayoutCompleted (LeechCraft::IHookProxy_ptr proxy,
			QWebPage *page, QWebFrame *frame)
	{
		auto match = [frame] (const UserScript& us) { return us.MatchToPage (frame->url ().toString ()); };
		auto inject = [frame, this] (const UserScript& us) { us.Inject (frame, Proxy_); };

		apply_if (UserScripts_.begin (), UserScripts_.end (), match, inject);
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		Proxy_ = qobject_cast<IProxyObject*>(proxy);

		if (!Proxy_)
		{
			qWarning () << Q_FUNC_INFO
				<< "unable to cast"
				<< proxy
				<< "to IProxyObject";
		}
	}

	void Plugin::EditScript (int scriptIndex)
	{
		const UserScript& script = UserScripts_.at (scriptIndex);
		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Poshuku_FatApe");
		const QString& editor = settings.value ("editor").toString ();

		if (editor.isEmpty ())
			return;

		QProcess::execute (editor, QStringList (script.Path ()));
	}

	void Plugin::DeleteScript (int scriptIndex)
	{
		UserScripts_ [scriptIndex].Delete ();
		UserScripts_.removeAt (scriptIndex);
	}

	void Plugin::SetScriptEnabled (int scriptIndex, bool value)
	{
		UserScripts_ [scriptIndex].SetEnabled (value);
	}

	void Plugin::hookAcceptNavigationRequest (LeechCraft::IHookProxy_ptr proxy, QWebPage *page,
			QWebFrame *frame, QNetworkRequest request, QWebPage::NavigationType type)
	{
		if (!request.url ().path ().endsWith ("user.js", Qt::CaseInsensitive) ||
				request.url ().scheme () == "file")
			return;

		UserScriptInstallerDialog installer (this,
				CoreProxy_->GetNetworkAccessManager (), request.url ());

		switch (installer.exec ())
		{
		case UserScriptInstallerDialog::Install:
			UserScripts_.append (UserScript (installer.TempScriptPath ()));
			UserScripts_.last ().Install (CoreProxy_->GetNetworkAccessManager ());
			AddScriptToManager (UserScripts_.last ());
			break;
		case UserScriptInstallerDialog::ShowSource:
			Proxy_->OpenInNewTab (QUrl::fromLocalFile (installer.TempScriptPath ()));
			break;
		case UserScriptInstallerDialog::Cancel:
			QFile::remove (installer.TempScriptPath ());
			break;
		default:
			break;
		}

		proxy->CancelDefault ();
	}

	void Plugin::AddScriptToManager (const UserScript& script)
	{
		QString scriptDesc = script.Description ();
		QStandardItem* name = new QStandardItem (script.Name ());
		QStandardItem* description = new QStandardItem (scriptDesc);

		name->setEditable (false);
		name->setData (script.IsEnabled (), EnabledRole);
		description->setEditable (false);
		WrapText (scriptDesc);
		description->setToolTip (scriptDesc);
		description->setData (script.IsEnabled (), EnabledRole);

		QList<QStandardItem*> items;
		items << name << description;
		Model_->appendRow (items);
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_poshuku_fatape, LeechCraft::Poshuku::FatApe::Plugin);
