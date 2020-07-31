/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSet>
#include <interfaces/core/icoreproxy.h>

class QModelIndex;

namespace LC
{
namespace LackMan
{
	class PackagesModel;

	class UpdatesNotificationManager : public QObject
	{
		Q_OBJECT

		PackagesModel * const PM_;
		const ICoreProxy_ptr Proxy_;

		QSet<int> UpgradablePackages_;

		bool NotifyScheduled_;
	public:
		UpdatesNotificationManager (PackagesModel*, ICoreProxy_ptr, QObject* = 0);

		bool HasUpgradable () const;
	private:
		void ScheduleNotify ();
	private slots:
		void handleDataChanged (const QModelIndex&, const QModelIndex&);
		void notify ();
	signals:
		void openLackmanRequested ();
		void hasUpgradablePackages (bool);
	};
}
}
