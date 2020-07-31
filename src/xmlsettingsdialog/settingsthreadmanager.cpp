/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "settingsthreadmanager.h"
#include <QMetaObject>
#include <QThread>
#include "settingsthread.h"
#include "basesettingsmanager.h"

namespace LC
{
	SettingsThreadManager::SettingsThreadManager ()
	: Thread_ { new QThread { this } }
	, Worker_ { std::make_shared<SettingsThread> () }
	{
		Thread_->start (QThread::IdlePriority);
		Worker_->moveToThread (Thread_);
	}

	SettingsThreadManager::~SettingsThreadManager ()
	{
		Thread_->quit ();

		if (Thread_->isRunning () && !Thread_->wait (10000))
			Thread_->terminate ();
	}

	SettingsThreadManager& SettingsThreadManager::Instance ()
	{
		static SettingsThreadManager stm;
		return stm;
	}

	void SettingsThreadManager::Add (Util::BaseSettingsManager *bsm,
			const QString& name, const QVariant& value)
	{
		Worker_->Save (bsm, name, value);
	}
}
