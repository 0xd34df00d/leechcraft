/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "updatesnotificationmanager.h"
#include <QTimer>
#include <util/xpc/util.h>
#include <util/models/modeliterator.h>
#include <util/xpc/notificationactionhandler.h>
#include <util/util.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/an/constants.h>
#include "packagesmodel.h"
#include "core.h"

namespace LC
{
namespace LackMan
{
	UpdatesNotificationManager::UpdatesNotificationManager (PackagesModel *model,
			ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, PM_ (model)
	, Proxy_ (proxy)
	, NotifyScheduled_ (false)
	{
		connect (PM_,
				SIGNAL (dataChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleDataChanged (QModelIndex, QModelIndex)));
		if (const auto rc = PM_->rowCount ())
			handleDataChanged (PM_->index (0, 0), PM_->index (rc - 1, 0));
	}

	bool UpdatesNotificationManager::HasUpgradable () const
	{
		return !UpgradablePackages_.isEmpty ();
	}

	void UpdatesNotificationManager::ScheduleNotify ()
	{
		if (NotifyScheduled_)
			return;

		NotifyScheduled_ = true;
		QTimer::singleShot (5000,
				this,
				SLOT (notify ()));
	}

	void UpdatesNotificationManager::handleDataChanged (const QModelIndex& from, const QModelIndex& to)
	{
		bool changed = false;
		for (int i = from.row (); i <= to.row (); ++i)
		{
			const auto& idx = PM_->index (i, 0);
			const auto id = idx.data (PackagesModel::PMRPackageID).toInt ();
			const auto upgradable = idx.data (PackagesModel::PMRUpgradable).toBool ();

			if (UpgradablePackages_.contains (id) && !upgradable)
			{
				UpgradablePackages_.remove (id);
				changed = true;
			}
			else if (!UpgradablePackages_.contains (id) && upgradable)
			{
				UpgradablePackages_ << id;
				changed = true;
			}
		}

		if (changed)
			ScheduleNotify ();
	}

	void UpdatesNotificationManager::notify ()
	{
		NotifyScheduled_ = false;

		auto em = Proxy_->GetEntityManager ();

		const auto upgradableCount = UpgradablePackages_.size ();

		emit hasUpgradablePackages (upgradableCount);

		QString bodyText;
		if (!upgradableCount)
		{
			auto cancel = Util::MakeANCancel ("org.LeechCraft.LackMan", "org.LeechCraft.LackMan");
			em->HandleEntity (cancel);

			return;
		}

		else if (upgradableCount <= 3)
		{
			QStringList names;
			for (auto id : UpgradablePackages_)
				names << Core::Instance ().GetListPackageInfo (id).Name_;
			names.sort ();

			if (upgradableCount == 1)
				bodyText = tr ("A new version of %1 is available.")
						.arg ("<em>" + names.value (0) + "</em>");
			else
			{
				const auto& lastName = names.takeLast ();
				bodyText = tr ("New versions of %1 and %2 are available.")
						.arg ("<em>" + names.join ("</em>, <em>") + "</em>")
						.arg ("<em>" + lastName + "</em>");
			}
		}
		else
			bodyText = tr ("New versions are available for %n package(s).",
					0, upgradableCount);

		auto entity = Util::MakeAN ("Lackman",
				bodyText,
				Priority::Info,
				"org.LeechCraft.LackMan",
				AN::CatPackageManager,
				AN::TypePackageUpdated,
				"org.LeechCraft.LackMan",
				{ "Lackman" },
				0,
				upgradableCount);

		auto nah = new Util::NotificationActionHandler (entity, this);
		nah->AddFunction (tr ("Open LackMan"),
				[this, entity, em] () -> void
				{
					emit openLackmanRequested ();
					em->HandleEntity (Util::MakeANCancel (entity));
				});

		em->HandleEntity (entity);
	}
}
}
