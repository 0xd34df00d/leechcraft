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

namespace LC::Kinotify
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

		QList<KinotifyWidget*> ActiveNotifications_;
		Util::XmlSettingsDialog_ptr SettingsDialog_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		EntityTestHandleResult CouldHandle (const Entity&) const override;
		void Handle (Entity) override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;
	private:
		void TestNotification ();
	};
}
