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
#include <interfaces/ientityhandler.h>
#include <interfaces/ihavesettings.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

namespace LC
{
namespace Util
{
	class ResourceLoader;
}

namespace Kinotify
{
	class KinotifyWidget;

	class Plugin : public QObject
					, public IInfo
					, public IEntityHandler
					, public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IEntityHandler IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.Kinotify")

		ICoreProxy_ptr Proxy_;

		QList<KinotifyWidget*> ActiveNotifications_;

		Util::XmlSettingsDialog_ptr SettingsDialog_;
		std::shared_ptr<Util::ResourceLoader> ThemeLoader_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
	private:
		void TestNotification ();
	public slots:
		void pushNotification ();
	private slots:
		void handleWatchedDirsChanged ();
	};
}
}
