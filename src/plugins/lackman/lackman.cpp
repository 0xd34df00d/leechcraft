/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lackman.h"
#include <QSortFilterProxyModel>
#include <QIcon>
#include <util/util.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/sll/slotclosure.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/entitytesthandleresult.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "lackmantab.h"

namespace LC
{
namespace LackMan
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("lackman");

		TabClass_.TabClass_ = "Lackman";
		TabClass_.VisibleName_ = "LackMan";
		TabClass_.Description_ = GetInfo ();
		TabClass_.Icon_ = GetIcon ();
		TabClass_.Priority_ = 0;
		TabClass_.Features_ = TFSingle | TFByDefault | TFOpenableByRequest;

		ShortcutMgr_ = new Util::ShortcutManager (proxy, this);

		SettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), "lackmansettings.xml");

		Core::Instance ().SetProxy (proxy);
		Core::Instance ().FinishInitialization ();

		SettingsDialog_->SetDataSource ("RepositoryList",
				Core::Instance ().GetRepositoryModel ());

		connect (&Core::Instance (),
				SIGNAL (gotEntity (LC::Entity)),
				this,
				SIGNAL (gotEntity (LC::Entity)));
	}

	void Plugin::SecondInit ()
	{
		Core::Instance ().SecondInit ();

		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this] { TabOpenRequested (TabClass_.TabClass_); },
			&Core::Instance (),
			SIGNAL (openLackmanRequested ()),
			this
		};
	}

	void Plugin::Release ()
	{
		Core::Instance ().Release ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LackMan";
	}

	QString Plugin::GetName () const
	{
		return "LackMan";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("LeechCraft Package Manager.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return { TabClass_ };
	}

	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		TabOpenRequested (tabClass, {});
	}

	void Plugin::TabOpenRequested (const QByteArray& tabClass, const QList<QPair<QByteArray, QVariant>>& props)
	{
		if (tabClass == "Lackman")
		{
			if (LackManTab_)
			{
				emit LackManTab_->raise ();
				return;
			}

			LackManTab_ = new LackManTab (ShortcutMgr_, TabClass_, this);
			for (const auto& pair : props)
				LackManTab_->setProperty (pair.first, pair.second);
			GetProxyHolder ()->GetRootWindowsManager ()->AddTab (GetName (), LackManTab_);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tabClass;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& entity) const
	{
		if (entity.Mime_ != "x-leechcraft/package-manager-action")
			return {};

		return EntityTestHandleResult (EntityTestHandleResult::PIdeal);
	}

	void Plugin::Handle (Entity entity)
	{
		const auto& action = entity.Entity_.toString ();
		if (action == "ListPackages")
		{
			TabOpenRequested ("Lackman");

			const auto& tags = entity.Additional_ ["Tags"].toStringList ();
			if (!tags.isEmpty ())
				LackManTab_->SetFilterTags (tags);
			else
				LackManTab_->SetFilterString (entity.Additional_ ["FilterString"].toString ());
		}
	}

	void Plugin::SetShortcut (const QString& id, const QKeySequences_t& seqs)
	{
		ShortcutMgr_->SetShortcut (id, seqs);
	}

	QMap<QString, ActionInfo> Plugin::GetActionInfo () const
	{
		return ShortcutMgr_->GetActionInfo ();
	}

	void Plugin::RecoverTabs (const QList<TabRecoverInfo>& infos)
	{
		for (const auto& recInfo : infos)
			if (recInfo.Data_ == "lackmantab")
				TabOpenRequested (TabClass_.TabClass_, recInfo.DynProperties_);
			else
				qWarning () << Q_FUNC_INFO
						<< "unknown context"
						<< recInfo.Data_;
	}

	bool Plugin::HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const
	{
		return true;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_lackman, LC::LackMan::Plugin);
