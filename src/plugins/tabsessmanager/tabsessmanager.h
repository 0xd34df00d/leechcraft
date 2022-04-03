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
#include <interfaces/iactionsexporter.h>
#include <interfaces/ishutdownlistener.h>
#include <interfaces/core/ihookproxy.h>

namespace LC::TabSessManager
{
	class SessionMenuManager;
	class SessionsManager;
	class UncloseManager;
	class TabsPropsManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IActionsExporter
				 , public IShutdownListener
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IActionsExporter IShutdownListener)

		LC_PLUGIN_METADATA ("org.LeechCraft.TabSessManager")

		struct Managers;
		std::shared_ptr<Managers> Mgrs_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		QList<QAction*> GetActions (ActionsEmbedPlace) const override;

		void HandleShutdownInitiated () override;
	public slots:
		void hookTabIsRemoving (const LC::IHookProxy_ptr& proxy,
				int index,
				int windowId);
		void hookTabAdding (const LC::IHookProxy_ptr& proxy,
				QWidget *widget);
		void hookGetPreferredWindowIndex (const LC::IHookProxy_ptr& proxy,
				const QWidget *widget) const;
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace) override;
	};
}
