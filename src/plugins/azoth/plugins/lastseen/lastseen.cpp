/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lastseen.h"
#include <QSettings>
#include <QCoreApplication>
#include <QIcon>
#include <QTimer>
#include <util/util.h>
#include <util/db/dblock.h>
#include <util/sll/qtutil.h>
#include <util/sll/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/iloadprogressreporter.h>
#include <interfaces/azoth/iclentry.h>
#include "ondiskstorage.h"
#include "entrystats.h"

namespace LC
{
namespace Azoth
{
namespace LastSeen
{
	using LastHash_t = QHash<QString, QDateTime>;
}
}
}

Q_DECLARE_METATYPE (LC::Azoth::LastSeen::LastHash_t);

namespace LC
{
namespace Azoth
{
namespace LastSeen
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("azoth_lastseen");

		qRegisterMetaType<LastHash_t> ("LC::Azoth::LastSeen::LastHash_t");
		qRegisterMetaTypeStreamOperators<LastHash_t> ("LC::Azoth::LastSeen::LastHash_t");

		Storage_ = std::make_shared<OnDiskStorage> ();

		Migrate (proxy->GetPluginsManager ());
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.LastSeen";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth LastSeen";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Azoth LastSeen displays when a contact has been online and available for the last time.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	namespace
	{
		bool IsGoodEntry (QObject *entryObj)
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
			if (!entry)
			{
				qWarning () << Q_FUNC_INFO
						<< entryObj
						<< "doesn't implement ICLEntry";
				return false;
			}

			return true;
		}
	}

	void Plugin::Migrate (IPluginsManager *manager)
	{
		QSettings settings
		{
			QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Azoth_LastSeen"
		};

		if (settings.allKeys ().isEmpty ())
			return;

		const auto& reporter = manager->CreateLoadProgressReporter (this);

		qDebug () << Q_FUNC_INFO
				<< "gonna migrate";

		const auto& avail = settings.value ("LastAvailable").value<LastHash_t> ();
		const auto& online = settings.value ("LastOnline").value<LastHash_t> ();
		const auto& status = settings.value ("LastStatusChange").value<LastHash_t> ();
		qDebug () << "done reading";

		QHash<QString, EntryStats> stats;

		{
			const auto& proc = reporter->InitiateProcess (tr ("Uniting keys"), 0, 3);

			for (const auto& pair : Util::Stlize (avail))
				stats [pair.first].Available_ = pair.second;

			++*proc;

			for (const auto& pair : Util::Stlize (online))
				stats [pair.first].Online_ = pair.second;

			++*proc;

			for (const auto& pair : Util::Stlize (status))
				stats [pair.first].StatusChange_ = pair.second;

			++*proc;
		}

		qDebug () << "done uniting";

		{
			auto lock = Storage_->BeginTransaction ();

			const auto& proc = reporter->InitiateProcess (tr ("Writing the database"), 0, stats.size ());
			for (const auto& pair : Util::Stlize (stats))
			{
				Storage_->SetEntryStats (pair.first, pair.second);
				++*proc;
			}

			qDebug () << "done writing";

			lock.Good ();

			qDebug () << "done committing";
		}

		settings.clear ();

		qDebug () << "done clearing";
	}

	void Plugin::hookEntryStatusChanged (IHookProxy_ptr, QObject *entryObj, QString variant)
	{
		if (!IsGoodEntry (entryObj))
			return;

		if (variant.isEmpty ())
			return;

		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		const auto& id = entry->GetEntryID ();
		const auto& status = entry->GetStatus ();

		if (!LastState_.contains (id))
		{
			LastState_ [id] = status.State_;
			return;
		}

		const State oldState = LastState_ [id];
		if (oldState == status.State_)
			return;

		LastState_ [id] = status.State_;

		const auto& now = QDateTime::currentDateTime ();

		auto stats = Storage_->GetEntryStats (id).value_or (EntryStats {});
		stats.StatusChange_ = now;

		switch (oldState)
		{
		case SOffline:
		case SProbe:
		case SError:
		case SInvalid:
		case SConnecting:
			return;
		case SOnline:
			stats.Available_ = now;
			[[fallthrough]];
		default:
			stats.Online_ = now;
			break;
		}

		QTimer::singleShot (0, [stats, st = Storage_, id] { st->SetEntryStats (id, stats); });
	}

	void Plugin::hookTooltipBeforeVariants (IHookProxy_ptr proxy, QObject *entryObj)
	{
		if (!IsGoodEntry (entryObj))
			return;

		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		const auto& id = entry->GetEntryID ();

		const auto& maybeStats = Storage_->GetEntryStats (id);
		if (!maybeStats)
			return;

		const auto& stats = *maybeStats;

		QString addition;

		const auto curState = entry->GetStatus ().State_;

		if (curState != SOnline)
		{
			const auto& avail = stats.Available_;
			if (avail.isValid ())
				addition += tr ("Was available: %1")
					.arg (avail.toString ());
		}

		if (curState == SOffline ||
				curState == SError ||
				curState == SInvalid)
		{
			const auto& online = stats.Online_;
			if (online.isValid ())
			{
				if (!addition.isEmpty ())
					addition += "<br/>";
				addition += tr ("Was online: %1")
					.arg (online.toString ());
			}
		}

		const auto& lastChange = stats.StatusChange_;
		if (lastChange.isValid ())
		{
			if (!addition.isEmpty ())
				addition += "<br/>";
			addition += tr ("Last status change: %1")
					.arg (lastChange.toString ());
		}

		if (addition.isEmpty ())
			return;

		const auto& tip = proxy->GetValue ("tooltip").toString ();
		proxy->SetValue ("tooltip", tip + "<br/><br/>" + addition + "<br/>");
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_lastseen, LC::Azoth::LastSeen::Plugin);
