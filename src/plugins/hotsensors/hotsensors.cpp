/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "hotsensors.h"
#include <QIcon>
#include <QAbstractItemModel>
#include <util/sll/qtutil.h>

#ifdef Q_OS_LINUX
#include "lmsensorsbackend.h"
#elif defined (Q_OS_MAC)
#include "macosbackend.h"
#endif

#include "historymanager.h"

namespace LC::HotSensors
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		HistoryMgr_ = std::make_unique<HistoryManager> ();

#ifdef Q_OS_LINUX
		SensorsMgr_ = std::make_unique<LmSensorsBackend> ();
#elif defined (Q_OS_MAC)
		SensorsMgr_ = std::make_unique<MacOsBackend> ();
#endif

		if (SensorsMgr_)
			connect (SensorsMgr_.get (),
					&Backend::gotReadings,
					HistoryMgr_.get (),
					&HistoryManager::HandleReadings);

		PlotMgr_ = std::make_unique<PlotManager> ();
		connect (HistoryMgr_.get (),
				&HistoryManager::historyChanged,
				PlotMgr_.get (),
				&PlotManager::Replot);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.HotSensors";
	}

	void Plugin::Release ()
	{
		SensorsMgr_.reset ();
	}

	QString Plugin::GetName () const
	{
		return "HotSensors"_qs;
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Temperature sensors information quark.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QuarkComponents_t Plugin::GetComponents () const
	{
		auto component = std::make_shared<QuarkComponent> ("hotsensors"_qs, "HSQuark.qml"_qs);
		component->ContextProps_.emplace_back ("HS_plotManager"_qs, PlotMgr_->CreateContextWrapper ());
		return { component };
	}
}


LC_EXPORT_PLUGIN (leechcraft_hotsensors, LC::HotSensors::Plugin);
