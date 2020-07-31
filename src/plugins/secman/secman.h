/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Vadim Misbakh-Soloviev, Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ipluginready.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ipersistentstorageplugin.h>

namespace LC
{
namespace SecMan
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPluginReady
				 , public IActionsExporter
				 , public IPersistentStoragePlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPluginReady IActionsExporter IPersistentStoragePlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.SecMan")

		QMap<QString, QList<QAction*>> MenuActions_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);

		QList<QAction*> GetActions (ActionsEmbedPlace area) const;
		QMap<QString, QList<QAction*>> GetMenuActions () const;

		IPersistentStorage_ptr RequestStorage ();
	private slots:
		void handleDisplayContents ();
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace);
	};
}
}
