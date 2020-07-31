/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "otlozhu.h"
#include <QIcon>
#include <util/util.h>
#include <util/sll/prelude.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/itagsmanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

#ifdef ENABLE_SYNC
#include "syncproxy.h"
#endif

#include "todotab.h"
#include "core.h"
#include "todomanager.h"
#include "todostorage.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Otlozhu
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("otlozhu");

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "otlozhusettings.xml");

		Core::Instance ().SetProxy (proxy);

		TCTodo_ = TabClassInfo
		{
			GetUniqueID () + "_todo",
			GetName (),
			GetInfo (),
			GetIcon (),
			20,
			TFOpenableByRequest | TFSingle | TFSuggestOpening
		};

#ifdef ENABLE_SYNC
		SyncProxy_ = new SyncProxy;
#endif
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Otlozhu";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Otlozhu";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("A simple GTD-compatible ToDo manager.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return { TCTodo_ };
	}

	void Plugin::TabOpenRequested (const QByteArray& id)
	{
		if (id == TCTodo_.TabClass_)
		{
			auto tab = new TodoTab (TCTodo_, this);
			emit addNewTab (TCTodo_.VisibleName_, tab);
			emit raiseTab (tab);

			connect (tab,
					SIGNAL (removeTab (QWidget*)),
					this,
					SIGNAL (removeTab (QWidget*)));

			connect (tab,
					SIGNAL (gotEntity (LC::Entity)),
					this,
					SIGNAL (gotEntity (LC::Entity)));
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown id"
					<< id;
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		return e.Mime_ == "x-leechcraft/todo-item" ?
				EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
				EntityTestHandleResult ();
	}

	void Plugin::Handle (Entity e)
	{
		auto mgr = Core::Instance ().GetTodoManager ();

		TodoItem_ptr item (new TodoItem ());
		item->SetTitle (e.Entity_.toString ());
		item->SetComment (e.Additional_ ["TodoBody"].toString ());

		const auto& tags = e.Additional_ ["Tags"].toStringList ();
		auto tm = Core::Instance ().GetProxy ()->GetTagsManager ();
		item->SetTagIDs (Util::Map (tags, [tm] (const QString& tag) { return tm->GetID (tag); }));

		mgr->GetTodoStorage ()->AddItem (item);
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

#ifdef ENABLE_SYNC
	ISyncProxy* Plugin::GetSyncProxy ()
	{
		return SyncProxy_;
	}
#endif
}
}

LC_EXPORT_PLUGIN (leechcraft_otlozhu, LC::Otlozhu::Plugin);
