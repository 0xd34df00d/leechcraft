/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "historyholder.h"
#include <QIcon>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include "historyentry.h"
#include "findproxy.h"
#include "historydb.h"

namespace LC
{
namespace HistoryHolder
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		qRegisterMetaType<HistoryEntry> ("LC::Plugins::HistoryHolder::Core::HistoryEntry");
#if QT_VERSION_MAJOR == 5
		qRegisterMetaTypeStreamOperators<HistoryEntry> ("LC::Plugins::HistoryHolder::Core::HistoryEntry");
#endif

		DB_ = std::make_shared<HistoryDB> (proxy->GetTagsManager ());
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.HistoryHolder";
	}

	QString Plugin::GetName () const
	{
		return "History holder";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Holds downloads history from various plugins.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QStringList Plugin::Provides () const
	{
		return { "history" };
	}

	QStringList Plugin::GetCategories () const
	{
		return { "history" };
	}

	QList<IFindProxy_ptr> Plugin::GetProxy (const Request& r)
	{
		return { std::make_shared<FindProxy> (DB_->CreateModel (), r) };
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		DB_->Add (e);
		return {};
	}

	void Plugin::Handle (LC::Entity)
	{
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_historyholder, LC::HistoryHolder::Plugin);
