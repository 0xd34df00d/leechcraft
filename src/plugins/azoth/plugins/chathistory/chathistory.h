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

#ifndef PLUGINS_AZOTH_PLUGINS_CHATHISTORY_CHATHISTORY_H
#define PLUGINS_AZOTH_PLUGINS_CHATHISTORY_CHATHISTORY_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/imessage.h>
#include <interfaces/ihistoryplugin.h>
#include <interfaces/core/ihookproxy.h>
#include "core.h"

class QTranslator;

namespace LeechCraft
{
namespace Azoth
{
namespace ChatHistory
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IActionsExporter
				 , public IHaveTabs
				 , public IHistoryPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				IActionsExporter
				IHaveTabs
				LeechCraft::Azoth::IHistoryPlugin)

		boost::shared_ptr<STGuard<Core> > Guard_;
		boost::shared_ptr<QTranslator> Translator_;
		QAction *ActionHistory_;
		QHash<QObject*, QAction*> Entry2ActionHistory_;
		QHash<QObject*, QAction*> Entry2ActionEnableHistory_;

		QHash<QString, QHash<QString, QObject*> > RequestedLogs_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		// IPlugin2
		QSet<QByteArray> GetPluginClasses () const;

		// IActionsExporter
		QList<QAction*> GetActions (ActionsEmbedPlace) const;
		QMap<QString, QList<QAction*> > GetMenuActions () const;

		// IHaveTabs
		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		// IHistoryPlugin
		bool IsHistoryEnabledFor (QObject*) const;
		void RequestLastMessages (QObject*, int);
	public slots:
		void initPlugin (QObject*);

		void hookEntryActionAreasRequested (LeechCraft::IHookProxy_ptr proxy,
				QObject *action,
				QObject *entry);
		void hookEntryActionsRequested (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry);
		void hookGotMessage2 (LeechCraft::IHookProxy_ptr proxy,
				QObject *message);
	private slots:
		void handleGotChatLogs (const QString&,
				const QString&, int, int, const QVariant&);

		void handleHistoryRequested ();
		void handleEntryHistoryRequested ();
		void handleEntryEnableHistoryRequested (bool);
		void handleEntryDestroyed ();
	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);

		void gotLastMessages (QObject*, const QList<QObject*>&);
	};
}
}
}

#endif
