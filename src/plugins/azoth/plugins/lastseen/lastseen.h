/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QDateTime>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/azoth/azothcommon.h>

class IPluginsManager;

namespace LC
{
namespace Azoth
{
namespace LastSeen
{
	class OnDiskStorage;
	struct EntryStats;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.LastSeen")

		QHash<QString, State> LastState_;

		std::shared_ptr<OnDiskStorage> Storage_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;
	private:
		void Migrate (IPluginsManager*);
	public slots:
		void hookEntryStatusChanged (LC::IHookProxy_ptr proxy,
				QObject *entry,
				QString variant);
		void hookTooltipBeforeVariants (LC::IHookProxy_ptr proxy,
				QObject *entry);
	};
}
}
}
