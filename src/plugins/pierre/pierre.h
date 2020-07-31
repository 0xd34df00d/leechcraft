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

namespace LC
{
namespace Pierre
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)

		LC_PLUGIN_METADATA ("org.LeechCraft.Pierre")

		QMenuBar *MenuBar_;
		ICoreProxy_ptr Proxy_;

		QMenu *TrayIconMenu_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;
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
}

