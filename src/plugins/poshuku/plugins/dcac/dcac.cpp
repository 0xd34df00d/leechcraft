/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dcac.h"
#include <QIcon>
#include <QMenu>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/poshuku/ibrowserwidget.h>
#include <interfaces/poshuku/iwebview.h>
#include "viewsmanager.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		ViewsManager_ = new ViewsManager { proxy->GetPluginsManager () };

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "poshukudcacsettings.xml");
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
		delete ViewsManager_;
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.DCAC";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku DC/AC";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Inverts the colors of web pages.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	void Plugin::hookBrowserWidgetInitialized (IHookProxy_ptr,
			QObject *browser)
	{
		const auto webView = qobject_cast<IBrowserWidget*> (browser)->GetWebView ();
		ViewsManager_->AddView (webView->GetQWidget ());
	}

	void Plugin::hookWebViewContextMenu (IHookProxy_ptr,
			IWebView *view, const ContextMenuInfo&,
			QMenu *menu, WebViewCtxMenuStage menuBuildStage)
	{
		if (menuBuildStage == WVSAfterFinish)
			menu->addAction (ViewsManager_->GetEnableAction (view->GetQWidget ()));
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_dcac, LC::Poshuku::DCAC::Plugin);
