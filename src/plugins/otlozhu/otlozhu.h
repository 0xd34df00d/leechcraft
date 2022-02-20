/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>

#ifdef ENABLE_SYNC
#include <interfaces/isyncable.h>
#endif

#include <interfaces/ientityhandler.h>
#include <interfaces/ihavesettings.h>

namespace LC
{
namespace Otlozhu
{
	class SyncProxy;

	class Plugin : public QObject
					, public IInfo
					, public IHaveTabs
					, public IHaveSettings
					, public IEntityHandler
#ifdef ENABLE_SYNC
					, public ISyncable
#endif
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs IEntityHandler IHaveSettings)
#ifdef ENABLE_SYNC
		Q_INTERFACES (ISyncable)
#endif

		LC_PLUGIN_METADATA ("org.LeechCraft.Otlozhu")

		TabClassInfo TCTodo_;

		Util::XmlSettingsDialog_ptr XSD_;

#ifdef ENABLE_SYNC
		SyncProxy *SyncProxy_ = nullptr;
#endif
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

#ifdef ENABLE_SYNC
		ISyncProxy* GetSyncProxy ();
#endif
	signals:
		void gotEntity (const LC::Entity&);
	};
}
}
