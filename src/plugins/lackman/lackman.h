/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_LACKMAN_LACKMAN_H
#define PLUGINS_LACKMAN_LACKMAN_H
#include <QWidget>
#include <QTranslator>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ientityhandler.h>
#include "ui_lackman.h"

class QSortFilterProxyModel;
class QStringListModel;

namespace LeechCraft
{
namespace LackMan
{
	class TypeFilterProxyModel;
	class StringFilterModel;

	class Plugin : public QWidget
				 , public IInfo
				 , public IHaveTabs
				 , public ITabWidget
				 , public IHaveSettings
				 , public IActionsExporter
				 , public IEntityHandler
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs ITabWidget IHaveSettings IActionsExporter IEntityHandler)

		Ui::LackMan Ui_;
		std::auto_ptr<QTranslator> Translator_;
		StringFilterModel *FilterString_;
		TypeFilterProxyModel *TypeFilter_;
		Util::XmlSettingsDialog_ptr SettingsDialog_;
		QStringListModel *TagsModel_;

		QAction *UpdateAll_;
		QAction *UpgradeAll_;
		QAction *Apply_;
		QAction *Cancel_;
		QToolBar *Toolbar_;

		TabClassInfo TabClass_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QList<QAction*> GetActions (ActionsEmbedPlace) const;

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);
	private slots:
		void handleTagsUpdated (const QStringList&);
		void on_PackageStatus__currentIndexChanged (int);
		void handlePackageSelected (const QModelIndex&);
		void handleFetchListUpdated (const QList<int>&);
	private:
		void BuildActions ();
	signals:
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
		void gotEntity (const LeechCraft::Entity&);
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);
	};
}
}

#endif
