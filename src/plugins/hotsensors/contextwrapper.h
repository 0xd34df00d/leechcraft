/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPointer>
#include <interfaces/core/icoreproxy.h>

class QAbstractItemModel;

namespace LC
{
namespace Util
{
	class UnhideListViewBase;
}

namespace HotSensors
{
	class SensorsFilterModel;

	class ContextWrapper : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;

		QString Context_;
		SensorsFilterModel *Filter_;
		QPointer<Util::UnhideListViewBase> CurrentList_;
	public:
		ContextWrapper (QAbstractItemModel*, ICoreProxy_ptr, QObject* = nullptr);
	private:
		QStringList LoadHiddenNames () const;
		void SaveHiddenNames (const QStringList&) const;
	public slots:
		void setContext (const QString&);
		QObject* getModel () const;

		void sensorUnhideListRequested (int x, int y, const QRect&);
		void unhideSensor (const QString&);
		void hideSensor (const QString&);
	};
}
}
