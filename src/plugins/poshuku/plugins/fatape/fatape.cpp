/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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
#include <util/sys/paths.h>
#include <util/sll/slotclosure.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/poshuku/ibrowserwidget.h>
#include <interfaces/poshuku/iwebview.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "userscriptsmanagerwidget.h"
#include "userscriptinstallerdialog.h"

namespace LC
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

		Util::InstallTranslator ("poshuku_fatape");

		QDir scriptsDir (Util::CreateIfNotExists ("data/poshuku/fatape/scripts"));

		if (!scriptsDir.exists ())
			return;

		QStringList filter ("*.user.js");

		Model_ = std::make_shared<QStandardItemModel> ();
		Model_->setHorizontalHeaderLabels ({ tr ("Name"), tr ("Description") });
		for (const auto& scriptPath : scriptsDir.entryInfoList (filter, QDir::Files))
			AddScriptToManager (UserScript { scriptPath.filePath () });

		connect (Model_.get (),
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged (QStandardItem*)));

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
		return CoreProxy_->GetIconThemeManager ()->GetPluginIcon ();
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

	void Plugin::hookBrowserWidgetInitialized (LC::IHookProxy_ptr,
			QObject *browserWidget)
	{
		const auto ibw = qobject_cast<IBrowserWidget*> (browserWidget);
		const auto view = ibw->GetWebView ();
		const auto viewWidget = view->GetQWidget ();
		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[view, this]
			{
				const auto& url = view->GetUrl ().toString ();
				auto match = [url] (const UserScript& us) { return us.MatchToPage (url); };
				auto inject = [view, this] (const UserScript& us) { us.Inject (view, Proxy_); };

				apply_if (UserScripts_.begin (), UserScripts_.end (), match, inject);
			},
			viewWidget,
			SIGNAL (earliestViewLayout ()),
			viewWidget
		};
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
		const auto& script = UserScripts_.at (scriptIndex);
		const auto& editor = XmlSettingsManager::Instance ()->property ("editor").toString ();
		if (!editor.isEmpty ())
			QProcess::execute (editor, { script.Path () });
		else
			QDesktopServices::openUrl (QUrl::fromLocalFile (script.Path ()));
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

	void Plugin::hookAcceptNavigationRequest (LC::IHookProxy_ptr proxy,
			const QUrl& url, IWebView*, IWebView::NavigationType, bool)
	{
		if (!url.path ().endsWith ("user.js", Qt::CaseInsensitive) ||
				url.scheme () == "file")
			return;

		UserScriptInstallerDialog installer { this, CoreProxy_->GetNetworkAccessManager (), url };

		switch (installer.exec ())
		{
		case UserScriptInstallerDialog::Install:
		{
			UserScript script { installer.TempScriptPath () };
			script.Install (CoreProxy_->GetNetworkAccessManager ());
			AddScriptToManager (script);
			break;
		}
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

	int Plugin::AddScriptToManager (const UserScript& script)
	{
		UserScripts_ << script;

		QString scriptDesc = script.Description ();
		QStandardItem* name = new QStandardItem (script.Name ());
		QStandardItem* description = new QStandardItem (scriptDesc);

		name->setCheckState (script.IsEnabled () ? Qt::Checked : Qt::Unchecked);
		name->setEditable (false);
		name->setCheckable (true);
		description->setEditable (false);
		WrapText (scriptDesc);
		description->setToolTip (scriptDesc);

		Model_->appendRow ({ name, description });

		return UserScripts_.size () - 1;
	}

	void Plugin::handleItemChanged (QStandardItem *item)
	{
		if (item->column ())
			return;

		auto& script = UserScripts_ [item->row ()];
		const auto shouldEnable = item->checkState () == Qt::Checked;
		if (shouldEnable != script.IsEnabled ())
			script.SetEnabled (shouldEnable);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_fatape, LC::Poshuku::FatApe::Plugin);
