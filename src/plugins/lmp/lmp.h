/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

namespace LeechCraft
{
namespace LMP
{
	class PlayerTab;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveTabs
				 , public IHaveSettings
				 , public IEntityHandler
				 , public IActionsExporter
				 , public IHaveRecoverableTabs
				 , public IHaveShortcuts
				 , public IPluginReady
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IHaveTabs
				IHaveSettings
				IEntityHandler
				IActionsExporter
				IHaveRecoverableTabs
				IHaveShortcuts
				IPluginReady)

		TabClassInfo PlayerTC_;
		TabClassInfo ArtistBrowserTC_;

		PlayerTab *PlayerTab_;

		Util::XmlSettingsDialog_ptr XSD_;

		QAction *ActionRescan_;
		QAction *ActionCollectionStats_;

		QMap<QString, Entity> GlobAction2Entity_;
		QMap<QString, ActionInfo> GlobAction2Info_;
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

		void SetShortcut (const QString&, const QKeySequences_t&);
		QMap<QString, ActionInfo> GetActionInfo () const;

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject* plugin);
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

		void gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace);

		void gotEntity (const LeechCraft::Entity&);
	};
}
}
