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
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		EntityTestHandleResult CouldHandle (const Entity&) const override;
		void Handle (Entity) override;
	public slots:
		void initPlugin (QObject*);
		void hookCollectContactIcons (LC::IHookProxy_ptr, QObject*, QList<QIcon>&) const;
	private slots:
		void handleEnableTracolorChanged ();
	};
}
}
}
