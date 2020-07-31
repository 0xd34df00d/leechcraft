/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxy.h>

class QSocketNotifier;
class QStandardItemModel;
class QStandardItem;
class QAbstractItemModel;

struct udev;
struct udev_monitor;

namespace LC
{
namespace Devmon
{
	class UDevBackend : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;

		std::shared_ptr<udev> UDev_;
		std::shared_ptr<udev_monitor> Mon_;

		QSocketNotifier *Notifier_ = 0;

		QStandardItemModel *Model_;
	public:
		UDevBackend (ICoreProxy_ptr, QObject* = 0);

		QAbstractItemModel* GetModel () const;
	private:
		void EnumerateAll ();
		QStandardItem* FindItemForPath (const QString&) const;
	private slots:
		void handleSocket (int);
	};
}
}
