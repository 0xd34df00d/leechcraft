/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Azer Abdullaev
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
#include <interfaces/ientityhandler.h>

class QMenuBar;
class QMainWindow;

namespace LC
{
namespace Lads
{
	class Plugin : public QObject
					, public IInfo
					, public IPlugin2
					, public IActionsExporter
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IActionsExporter)
	private:
		QAction *Action_;
		QMenuBar *MenuBar_;
		ICoreProxy_ptr Proxy_;
		QMainWindow *MW_;
		bool UnityDetected_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		
		QSet<QByteArray> GetPluginClasses () const;

		QList<QAction*> GetActions (ActionsEmbedPlace) const;

	public slots:
		void showHideMain () const;
		void hookGonnaFillMenu (LC::IHookProxy_ptr);
	private slots:
		void fillMenu ();
		void handleGotActions (const QList<QAction*>&, LC::ActionsEmbedPlace);
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace);
	};
}
}

