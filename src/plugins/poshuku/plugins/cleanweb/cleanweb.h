/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QNetworkAccessManager>
#include <QMenu>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/istartupwizard.h>
#include <interfaces/poshuku/poshukutypes.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/core/ipluginsmanager.h>

class QContextMenuEvent;

namespace LC
{
namespace Poshuku
{
class IWebView;

namespace CleanWeb
{
	class Core;

	class CleanWeb : public QObject
					, public IInfo
					, public IHaveSettings
					, public IEntityHandler
					, public IStartupWizard
					, public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings IEntityHandler IStartupWizard IPlugin2)

		LC_PLUGIN_METADATA ("org.LeechCraft.Poshuku.CleanWeb")

		ICoreProxy_ptr Proxy_;

		std::shared_ptr<Core> Core_;

		Util::XmlSettingsDialog_ptr SettingsDialog_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		QStringList Needs () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);

		QList<QWizardPage*> GetWizardPages () const;

		QSet<QByteArray> GetPluginClasses () const;
	public slots:
		void hookBrowserWidgetInitialized (LC::IHookProxy_ptr proxy,
				QObject *browserWidget);
		void hookWebViewContextMenu (LC::IHookProxy_ptr,
				LC::Poshuku::IWebView*,
				const LC::Poshuku::ContextMenuInfo&, QMenu*,
				WebViewCtxMenuStage);
	};
}
}
}
