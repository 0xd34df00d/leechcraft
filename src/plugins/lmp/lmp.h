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
#include <interfaces/ihavetabs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <interfaces/ipluginready.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/ijobholder.h>
#include <interfaces/idatafilter.h>
#include <interfaces/ihavediaginfo.h>

namespace LC
{
namespace LMP
{
	class PlayerTab;
	class EffectsManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveTabs
				 , public IHaveSettings
				 , public IEntityHandler
				 , public IActionsExporter
				 , public IHaveRecoverableTabs
				 , public IHaveShortcuts
				 , public IPluginReady
				 , public IJobHolder
				 , public IDataFilter
				 , public IHaveDiagInfo
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IHaveTabs
				IHaveSettings
				IEntityHandler
				IActionsExporter
				IHaveRecoverableTabs
				IHaveShortcuts
				IPluginReady
				IJobHolder
				IDataFilter
				IHaveDiagInfo)

		LC_PLUGIN_METADATA ("org.LeechCraft.LMP")

		ICoreProxy_ptr Proxy_;

		TabClassInfo PlayerTC_;
		TabClassInfo ArtistBrowserTC_;

		PlayerTab *PlayerTab_;

		Util::XmlSettingsDialog_ptr XSD_;

		QAction *ActionRescan_;
		QAction *ActionCollectionStats_;

		QMap<QString, Entity> GlobAction2Entity_;
		QMap<QString, ActionInfo> GlobAction2Info_;

		EffectsManager *EffectsMgr_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);

		QList<QAction*> GetActions (ActionsEmbedPlace area) const;
		QMap<QString, QList<QAction*>> GetMenuActions () const;

		void RecoverTabs (const QList<TabRecoverInfo>& infos);
		bool HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const;

		void SetShortcut (const QString&, const QKeySequences_t&);
		QMap<QString, ActionInfo> GetActionInfo () const;

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject* plugin);

		QAbstractItemModel* GetRepresentation () const;

		QString GetFilterVerb () const;
		QList<FilterVariant> GetFilterVariants (const QVariant&) const;

		QString GetDiagInfoString () const;
	private:
		void InitShortcuts ();
	private slots:
		void handleFullRaiseRequested ();
		void showCollectionStats ();

		void handleArtistBrowseRequested (const QString&, const DynPropertiesList_t& = DynPropertiesList_t ());
	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);

		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace);
	};
}
}
