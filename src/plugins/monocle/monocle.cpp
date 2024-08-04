/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "monocle.h"
#include <QIcon>
#include <qurl.h>
#include <util/util.h>
#include <util/shortcuts/shortcutmanager.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/sll/qtutil.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "interfaces/monocle/ibackendplugin.h"
#include "interfaces/monocle/iredirectorplugin.h"
#include "util/monocle/textdocumentformatconfig.h"
#include "components/navigation/bookmarksstorage.h"
#include "components/services/docstatemanager.h"
#include "components/services/documentloader.h"
#include "components/services/recentlyopenedmanager.h"
#include "core.h"
#include "documenttab.h"
#include "xmlsettingsmanager.h"
#include "defaultbackendmanager.h"

namespace LC::Monocle
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		BookmarksStorage_ = std::make_shared<BookmarksStorage> ();
		DocStateManager_ = std::make_shared<DocStateManager> ();
		Loader_ = std::make_shared<DocumentLoader> ();
		RecentlyOpenedManager_ = std::make_shared<RecentlyOpenedManager> ();

		Util::InstallTranslator ("monocle");

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "monoclesettings.xml");

		TextDocumentFormatConfig::Instance ().SetXSM (XmlSettingsManager::Instance ());

		Core::Instance ().SetProxy (proxy, this);

		XSD_->SetDataSource ("DefaultBackends", Loader_->GetDefaultBackendManager ().GetModel ());

		DocTabInfo_ =
		{
			GetUniqueID () + "_Document",
			"Monocle",
			GetInfo (),
			GetIcon (),
			55,
			TFOpenableByRequest | TFSuggestOpening
		};

		const auto sm = Core::Instance ().GetShortcutManager ();
		sm->RegisterActionInfo ("org.LeechCraft.Monocle.PrevAnn",
				{
					tr ("Go to previous annotation"),
					QKeySequence {},
					proxy->GetIconThemeManager ()->GetIcon ("go-previous")
				});
		sm->RegisterActionInfo ("org.LeechCraft.Monocle.NextAnn",
				{
					tr ("Go to next annotation"),
					QKeySequence {},
					proxy->GetIconThemeManager ()->GetIcon ("go-next")
				});
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Monocle";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Monocle"_qs;
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Modular document viewer for LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		if (!(e.Parameters_ & FromUserInitiated))
			return {};

		if (!e.Entity_.canConvert<QUrl> ())
			return {};

		const auto& url = e.Entity_.toUrl ();
		if (url.scheme () != "file"_qs)
			return {};

		const auto& local = url.toLocalFile ();
		if (!QFile::exists (local))
			return {};

		return Loader_->CanLoadDocument (local) ?
				EntityTestHandleResult { EntityTestHandleResult::PIdeal } :
				EntityTestHandleResult {};
	}

	namespace
	{
		void AddTab (DocumentTab *tab)
		{
			GetProxyHolder ()->GetRootWindowsManager ()->AddTab (tab->GetTabClassInfo ().VisibleName_, tab);
		}
	}

	void Plugin::Handle (Entity e)
	{
		auto tab = CreateTab ();
		AddTab (tab);
		tab->SetDoc (e.Entity_.toUrl ().toLocalFile ());
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return { DocTabInfo_ };
	}

	void Plugin::TabOpenRequested (const QByteArray& id)
	{
		if (id == DocTabInfo_.TabClass_)
			AddTab (CreateTab ());
		else
			qWarning () << "unknown tab class" << id;
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		return { IBackendPlugin::PluginClass, IRedirectorPlugin::PluginClass };
	}

	void Plugin::AddPlugin (QObject *pluginObj)
	{
		Loader_->AddPlugin (pluginObj);
	}

	void Plugin::RecoverTabs (const QList<TabRecoverInfo>& infos)
	{
		for (const auto& info : infos)
		{
			auto tab = CreateTab ();
			for (const auto& pair : info.DynProperties_)
				tab->setProperty (pair.first, pair.second);
			AddTab (tab);
			tab->RecoverState (info.Data_);
		}
	}

	bool Plugin::HasSimilarTab (const QByteArray& data, const QList<QByteArray>& other) const
	{
		return StandardSimilarImpl (data, other,
				[] (const QByteArray& data)
				{
					quint8 version = 0;
					QString path;
					QDataStream str { data };
					str >> version >> path;
					return std::make_tuple (version, path);
				});
	}

	QMap<QByteArray, ActionInfo> Plugin::GetActionInfo () const
	{
		return Core::Instance ().GetShortcutManager( )->GetActionInfo ();
	}

	void Plugin::SetShortcut (const QByteArray& id, const QKeySequences_t& sequences)
	{
		Core::Instance ().GetShortcutManager ()->SetShortcut (id, sequences);
	}

	DocumentTab* Plugin::CreateTab ()
	{
		return new DocumentTab
		{
			{ *BookmarksStorage_, *DocStateManager_, *Loader_, *RecentlyOpenedManager_, DocTabInfo_ },
			this
		};
	}
}

LC_EXPORT_PLUGIN (leechcraft_monocle, LC::Monocle::Plugin);
