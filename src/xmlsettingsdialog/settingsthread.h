/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMap>
#include <QVariant>
#include <QMutex>

namespace LC
{
namespace Util
{
	class BaseSettingsManager;
}

	class SettingsThread : public QObject
	{
		Q_OBJECT

		QMutex Mutex_;
		QMap<Util::BaseSettingsManager*, QList<QPair<QString, QVariant>>> Pendings_;
	public:
		using QObject::QObject;
		~SettingsThread ();

		void Save (Util::BaseSettingsManager*, QString, QVariant);
	private slots:
		void saveScheduled ();
	};
}
