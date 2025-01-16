/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cleanweb.h"
#include <QIcon>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/poshuku/ibrowserwidget.h>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "subscriptionsmanagerwidget.h"
#include "userfilters.h"
#include "wizardgenerator.h"
#include "subscriptionsmodel.h"
#include "userfiltersmodel.h"

namespace LC
{
namespace Poshuku
{
namespace CleanWeb
{
	void CleanWeb::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("poshuku_cleanweb");

		SettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"poshukucleanwebsettings.xml");

		const auto model = new SubscriptionsModel { this };
		const auto ufm = new UserFiltersModel { proxy, this };
		Core_ = std::make_shared<Core> (model, ufm, proxy);

		SettingsDialog_->SetCustomWidget ("SubscriptionsManager",
				new SubscriptionsManagerWidget (Core_.get (), model));
		SettingsDialog_->SetCustomWidget ("UserFilters", new UserFilters (ufm));
	}

	void CleanWeb::SecondInit ()
	{
		Core_->InstallInterceptor ();
	}

	void CleanWeb::Release ()
	{
	}

	QByteArray CleanWeb::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.CLeanWeb";
	}

	QString CleanWeb::GetName () const
	{
		return "Poshuku CleanWeb";
	}

	QString CleanWeb::GetInfo () const
	{
		return tr ("Blocks unwanted ads.");
	}

	QIcon CleanWeb::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QStringList CleanWeb::Needs () const
	{
		return { "http" };
	}

	Util::XmlSettingsDialog_ptr CleanWeb::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	EntityTestHandleResult CleanWeb::CouldHandle (const Entity& e) const
	{
		return Core_->CouldHandle (e) ?
				EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
				EntityTestHandleResult ();
	}

	void CleanWeb::Handle (Entity e)
	{
		Core_->Handle (e);
	}

	QList<QWizardPage*> CleanWeb::GetWizardPages () const
	{
		return WizardGenerator::GetPages (Core_.get ());
	}

	QSet<QByteArray> CleanWeb::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
	}

	void CleanWeb::hookBrowserWidgetInitialized (IHookProxy_ptr, QObject *browserWidget)
	{
		Core_->HandleBrowserWidget (qobject_cast<IBrowserWidget*> (browserWidget));
	}

	void CleanWeb::hookWebViewContextMenu (IHookProxy_ptr,
			IWebView *view,
			const ContextMenuInfo& r,
			QMenu *menu,
			WebViewCtxMenuStage stage)
	{
		Core_->HandleContextMenu (r, view, menu, stage);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_cleanweb, LC::Poshuku::CleanWeb::CleanWeb);
