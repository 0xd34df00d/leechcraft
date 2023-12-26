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
#include <util/sll/qtutil.h>
#include <util/sll/util.h>
#include <util/sys/fdguard.h>
#include "basesettingsmanager.h"

namespace LC
{
	SettingsThread::~SettingsThread ()
	{
		QMutexLocker l { &Mutex_ };
		if (!Pendings_.isEmpty ())
			qWarning () << Q_FUNC_INFO
					<< "there are pending settings to be saved, unfortunately they will be lost :(";
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

		for (const auto& pair : Util::Stlize (pendings))
		{
			const auto& s = pair.first->MakeSettings ();
			for (const auto& p : pair.second)
				s->setValue (p.first, p.second);
		}
	}
}
