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
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "documenttab.h"
#include "xmlsettingsmanager.h"
#include "defaultbackendmanager.h"

namespace LC
{
namespace Monocle
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("monocle");

		qRegisterMetaType<QList<int>> ("QList<int>");

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "monoclesettings.xml");

		Core::Instance ().SetProxy (proxy);

		XSD_->SetDataSource ("DefaultBackends",
				Core::Instance ().GetDefaultBackendManager ()->GetModel ());

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
		sm->SetObject (this);
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
		return "Monocle";
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
			return EntityTestHandleResult ();

		if (!e.Entity_.canConvert<QUrl> ())
			return EntityTestHandleResult ();

		const auto& url = e.Entity_.toUrl ();
		if (url.scheme () != "file")
			return EntityTestHandleResult ();

		const auto& local = url.toLocalFile ();
		if (!QFile::exists (local))
			return EntityTestHandleResult ();

		return Core::Instance ().CanLoadDocument (local) ?
				EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
				EntityTestHandleResult ();
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
		auto tab = new DocumentTab (DocTabInfo_, this);
		tab->SetDoc (e.Entity_.toUrl ().toLocalFile (), DocumentTab::DocumentOpenOptions {});
		AddTab (tab);
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
			AddTab (new DocumentTab (DocTabInfo_, this));
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< id;
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Monocle.IBackendPlugin";
		return result;
	}

	void Plugin::AddPlugin (QObject *pluginObj)
	{
		Core::Instance ().AddPlugin (pluginObj);
	}

	void Plugin::RecoverTabs (const QList<TabRecoverInfo>& infos)
	{
		for (const auto& info : infos)
		{
			auto tab = new DocumentTab (DocTabInfo_, this);
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

	QMap<QString, ActionInfo> Plugin::GetActionInfo () const
	{
		return Core::Instance ().GetShortcutManager( )->GetActionInfo ();
	}

	void Plugin::SetShortcut (const QString& id, const QKeySequences_t& sequences)
	{
		Core::Instance ().GetShortcutManager ()->SetShortcut (id, sequences);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_monocle, LC::Monocle::Plugin);

