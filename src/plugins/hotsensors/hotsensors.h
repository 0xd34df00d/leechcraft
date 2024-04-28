/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iquarkcomponentprovider.h>
#include "backend.h"
#include "plotmanager.h"

namespace LC
{
namespace HotSensors
{
	class Backend;
	class HistoryManager;
	class PlotManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IQuarkComponentProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IQuarkComponentProvider)

		LC_PLUGIN_METADATA ("org.LeechCraft.HotSensors")

		std::unique_ptr<Backend> SensorsMgr_;
		std::unique_ptr<HistoryManager> HistoryMgr_;
		std::unique_ptr<PlotManager> PlotMgr_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QuarkComponents_t GetComponents () const;
	};
}
}
