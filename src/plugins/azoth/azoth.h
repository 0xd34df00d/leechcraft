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

#ifndef PLUGINS_AZOTH_AZOTH_H
#define PLUGINS_AZOTH_AZOTH_H
#include <QObject>
#include <QTranslator>
#include <QModelIndex>
#include <interfaces/iinfo.h>
#include <interfaces/ipluginready.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ijobholder.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ianemitter.h>

namespace LeechCraft
{
namespace Azoth
{
	class MainWidget;
	class ConsoleWidget;

	class Plugin : public QObject
				 , public IInfo
				 , public IPluginReady
				 , public IHaveTabs
				 , public IHaveSettings
				 , public IJobHolder
				 , public IActionsExporter
				 , public IEntityHandler
				 , public IANEmitter
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPluginReady
				IHaveTabs
				IHaveSettings
				IJobHolder
				IActionsExporter
				IEntityHandler
				IANEmitter)

		MainWidget *MW_;
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
		std::auto_ptr<QTranslator> Translator_;
		TabClasses_t TabClasses_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		QStringList Provides () const;

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QAbstractItemModel* GetRepresentation () const;

		QList<QAction*> GetActions (ActionsEmbedPlace) const;
		QMap<QString, QList<QAction*> > GetMenuActions () const;

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		QList<ANFieldData> GetANFields () const;
	public slots:
		void handleTasksTreeSelectionCurrentRowChanged (const QModelIndex&, const QModelIndex&);
	private slots:
		void handleMoreThisStuff (const QString&);
		void handleConsoleWidget (ConsoleWidget*);
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);

		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);

		void gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace);
	};
}
}

#endif

