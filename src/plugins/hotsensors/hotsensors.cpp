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
#include <util/util.h>
#include <util/sys/paths.h>

#ifdef Q_OS_LINUX
#include "lmsensorsbackend.h"
#elif defined (Q_OS_MAC)
#include "macosbackend.h"
#endif

#include "historymanager.h"

namespace LC
{
namespace HotSensors
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("hotsensors");

		HistoryMgr_ = std::make_unique<HistoryManager> ();

#ifdef Q_OS_LINUX
		SensorsMgr_ = std::make_unique<LmSensorsBackend> ();
#elif defined (Q_OS_MAC)
		SensorsMgr_ = std::make_unique<MacOsBackend> ();
#endif

		if (SensorsMgr_)
			connect (SensorsMgr_.get (),
					SIGNAL (gotReadings (Readings_t)),
					HistoryMgr_.get (),
					SLOT (handleReadings (Readings_t)));

		PlotMgr_ = std::make_unique<PlotManager> ();
		connect (HistoryMgr_.get (),
				SIGNAL (historyChanged (ReadingsHistory_t)),
				PlotMgr_.get (),
				SLOT (handleHistoryUpdated (ReadingsHistory_t)));
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
		return "HotSensors";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Temperature sensors information quark.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QuarkComponents_t Plugin::GetComponents () const
	{
		auto component = std::make_shared<QuarkComponent> ("hotsensors", "HSQuark.qml");
		component->ContextProps_.push_back ({ "HS_plotManager", PlotMgr_->CreateContextWrapper () });
		return { component };
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_hotsensors, LC::HotSensors::Plugin);
