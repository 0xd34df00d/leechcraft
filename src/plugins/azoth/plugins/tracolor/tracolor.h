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
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/core/ihookproxy.h>

namespace LC
{
namespace Azoth
{
class IProxyObject;

namespace Tracolor
{
	class EntryEventsManager;
	class IconsManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
				 , public IEntityHandler
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				IHaveSettings
				IEntityHandler)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.Tracolor")

		EntryEventsManager *EventsManager_ = nullptr;
		IconsManager *IconsManager_ = nullptr;

		Util::XmlSettingsDialog_ptr XSD_;

		IProxyObject *AzothProxy_ = nullptr;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);
	public slots:
		void initPlugin (QObject*);
		void hookCollectContactIcons (LC::IHookProxy_ptr, QObject*, QList<QIcon>&) const;
	private slots:
		void handleIconsUpdated (const QByteArray&);
		void handleEnableTracolorChanged ();
	};
}
}
}

