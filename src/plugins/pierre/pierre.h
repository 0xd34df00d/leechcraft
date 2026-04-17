/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/iinfo.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/iplugin2.h>

class QMenuBar;
class QSystemTrayIcon;

namespace LC::Pierre
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)

		LC_PLUGIN_METADATA ("org.LeechCraft.Pierre")

		QMenuBar *MenuBar_ = nullptr;
		QMenu *TrayIconMenu_ = nullptr;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;
	public slots:
		void hookGonnaFillMenu (LC::IHookProxy_ptr);
		void hookTrayIconCreated (LC::IHookProxy_ptr,
				QSystemTrayIcon*);
		void hookTrayIconVisibilityChanged (LC::IHookProxy_ptr,
				QSystemTrayIcon*,
				bool);
	private slots:
		void handleGotActions (const QList<QAction*>&, LC::ActionsEmbedPlace);
		void handleWindow (int);
		void fillMenu ();
	};
}
