/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QModelIndex>
#include <interfaces/iinfo.h>
#include <interfaces/ipluginready.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ijobholder.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/an/ianemitter.h>

namespace LC
{
namespace Azoth
{
	class ServiceDiscoveryWidget;
	class MainWidget;
	class ConsoleWidget;
	class MicroblogsTab;
	class ServerHistoryWidget;

	class Plugin : public QObject
				 , public IInfo
				 , public IPluginReady
				 , public IHaveTabs
				 , public IHaveRecoverableTabs
				 , public IHaveSettings
				 , public IJobHolder
				 , public IActionsExporter
				 , public IEntityHandler
				 , public IHaveShortcuts
				 , public IANEmitter
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPluginReady
				IHaveTabs
				IHaveRecoverableTabs
				IHaveSettings
				IJobHolder
				IActionsExporter
				IEntityHandler
				IHaveShortcuts
				IANEmitter)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth")

		MainWidget *MW_;
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;

		TabClasses_t TabClasses_;

		QMenu *StatusChangeMenu_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;
		QStringList Provides () const override;

		QSet<QByteArray> GetExpectedPluginClasses () const override;
		void AddPlugin (QObject*) override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		QAbstractItemModel* GetRepresentation () const override;
		IJobHolderRepresentationHandler_ptr CreateRepresentationHandler () override;

		QList<QAction*> GetActions (ActionsEmbedPlace) const override;
		QMap<QString, QList<QAction*>> GetMenuActions () const override;

		EntityTestHandleResult CouldHandle (const Entity&) const override;
		void Handle (Entity) override;

		TabClasses_t GetTabClasses () const override;
		void TabOpenRequested (const QByteArray&) override;

		void RecoverTabs (const QList<TabRecoverInfo>&) override;
		bool HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const override;

		void SetShortcut (const QString&, const QKeySequences_t&) override;
		QMap<QString, ActionInfo> GetActionInfo() const override;

		QList<ANFieldData> GetANFields () const override;
	private :
		void InitShortcuts ();
		void InitAccActsMgr ();
		void InitSettings ();
		void InitMW ();
		void InitSignals ();
		void InitTabClasses ();
	public slots:
		void handleServerHistoryTab (ServerHistoryWidget*);
	private slots:
		void handleMWLocation (Qt::DockWidgetArea);
		void handleMWFloating (bool);
		void handleMoreThisStuff (const QString&);
	signals:
		void gotEntity (const LC::Entity&) override;

		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace) override;
	};
}
}
