/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_AGGREGATOR_AGGREGATOR_H
#define PLUGINS_AGGREGATOR_AGGREGATOR_H
#include <memory>
#include <QWidget>
#include <QItemSelection>
#include <interfaces/iinfo.h>
#include <interfaces/iembedtab.h>
#include <interfaces/ijobholder.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/structures.h>
#include <interfaces/itoolbarembedder.h>
#include <interfaces/itraymenu.h>
#include <interfaces/istartupwizard.h>

class QSystemTrayIcon;
class QTranslator;
class QToolBar;
class IDownload;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class ItemBucket;
			struct Enclosure;

			struct Aggregator_Impl;

			class Aggregator : public QWidget
							 , public IInfo
							 , public IEmbedTab
							 , public IHaveSettings
							 , public IJobHolder
							 , public IEntityHandler
							 , public IHaveShortcuts
							 , public IToolBarEmbedder
							 , public IStartupWizard
							 , public ITrayMenu
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IEmbedTab IHaveSettings IJobHolder IEntityHandler IHaveShortcuts IToolBarEmbedder IStartupWizard ITrayMenu)

				Aggregator_Impl *Impl_;
			public:
				virtual ~Aggregator ();
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				void Release ();
				QString GetName () const;
				QString GetInfo () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);
				QIcon GetIcon () const;

				QWidget* GetTabContents ();
				QToolBar* GetToolBar () const;
				QAbstractItemModel* GetRepresentation () const;

				boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> GetSettingsDialog () const;

				bool CouldHandle (const LeechCraft::DownloadEntity&) const;
				void Handle (LeechCraft::DownloadEntity);

				void SetShortcut (int, const QKeySequence&);
				QMap<int, LeechCraft::ActionInfo> GetActionInfo () const;

				QList<QWizardPage*> GetWizardPages () const;

				QList<QAction*> GetActions () const;

				QList<QAction*> GetTrayActions () const;
				QList<QMenu*> GetTrayMenus () const;
			protected:
				virtual void keyPressEvent (QKeyEvent*);
			private:
				void ScheduleShowError ();
				bool IsRepr () const;
				QModelIndex GetRelevantIndex () const;
			private slots:
				void handleItemSelected (const QModelIndex&);
				void on_ActionAddFeed__triggered ();
				void on_ActionRemoveFeed__triggered ();
				void on_ActionUpdateSelectedFeed__triggered ();
				void on_ActionItemBucket__triggered ();
				void on_ActionRegexpMatcher__triggered ();
				void on_ActionImportOPML__triggered ();
				void on_ActionExportOPML__triggered ();
				void on_ActionImportBinary__triggered ();
				void on_ActionExportBinary__triggered ();
				void on_ActionExportFB2__triggered ();
				void on_ActionMarkChannelAsRead__triggered ();
				void on_ActionMarkChannelAsUnread__triggered ();
				void on_ActionChannelSettings__triggered ();
				void on_MergeItems__toggled (bool);
				void on_ShowAsTape__toggled (bool);
				void currentChannelChanged ();
				void unreadNumberChanged (int);
				void trayIconActivated ();
				void handleGroupChannels ();
			signals:
				void notify (const LeechCraft::Notification&);
				void gotEntity (const LeechCraft::DownloadEntity&);
				void delegateEntity (const LeechCraft::DownloadEntity&,
						int*, QObject**);
				void bringToFront ();
			};
		};
	};
};

#endif

