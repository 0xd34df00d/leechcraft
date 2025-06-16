/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "settingsthread.h"
#include <QMutexLocker>
#include <QFile>
#include <QTimer>
#include <QtDebug>
#include <util/sll/util.h>
#include <util/sys/fdguard.h>
#include "basesettingsmanager.h"

namespace LC
{
	SettingsThread::~SettingsThread ()
	{
		QMutexLocker l { &Mutex_ };
		if (!Pendings_.isEmpty ())
			qWarning () << "there are pending settings to be saved, unfortunately they will be lost :(";
	}

	void SettingsThread::Save (Util::BaseSettingsManager *bsm, QString name, QVariant value)
	{
		QMutexLocker l { &Mutex_ };

		if (Pendings_.isEmpty ())
			QTimer::singleShot (0, this, SLOT (saveScheduled ()));
		Pendings_ [bsm].push_back ({ name, value });
	}

	void SettingsThread::saveScheduled ()
	{
		decltype (Pendings_) pendings;

		{
			QMutexLocker l { &Mutex_ };
			using std::swap;
			swap (pendings, Pendings_);
		}

		for (const auto& [settingsMgr, updates] : pendings.asKeyValueRange ())
		{
			const auto& s = settingsMgr->MakeSettings ();
			for (const auto& [key, value] : updates)
			{
				if (value.isValid ())
					s->setValue (key, value);
				else
					s->remove (key);
			}
		}
	}
}
