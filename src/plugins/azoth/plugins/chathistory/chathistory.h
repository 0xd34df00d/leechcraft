/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QAction>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/ihistoryplugin.h>
#include "storagestructures.h"

namespace LC
{
namespace Azoth
{
class IProxyObject;

namespace ChatHistory
{
	class ChatHistoryWidget;
	class StorageManager;
	class LoggingStateKeeper;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IActionsExporter
				 , public IHaveTabs
				 , public IHaveSettings
				 , public IHistoryPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				IActionsExporter
				IHaveTabs
				IHaveSettings
				LC::Azoth::IHistoryPlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.ChatHistory")

		TabClassInfo TabClass_;

		Util::XmlSettingsDialog_ptr XSD_;

		std::shared_ptr<LoggingStateKeeper> LoggingStateKeeper_;
		std::shared_ptr<StorageManager> StorageMgr_;
		QAction *ActionHistory_ = nullptr;
		QHash<QObject*, QAction*> Entry2ActionHistory_;
		QHash<QObject*, QAction*> Entry2ActionEnableHistory_;

		QAction *SeparatorAction_ = nullptr;

		ICoreProxy_ptr CoreProxy_;
		IProxyObject *PluginProxy_ = nullptr;
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
		QMap<QString, QList<QAction*>> GetMenuActions () const;

		// IHaveTabs
		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		// IHaveSettings
		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		// IHistoryPlugin
		bool IsHistoryEnabledFor (QObject*) const;
		void RequestLastMessages (QObject*, int);
		QFuture<MaxTimestampResult_t> RequestMaxTimestamp (IAccount*);
		void AddRawMessages (const QString&, const QString&, const QString&, const QList<HistoryItem>&);
	private:
		void HandleGotChatLogs (const QPointer<QObject>&, const ChatLogsResult_t&);

		void HandleHistoryRequested ();
		void HandleEntryHistoryRequested (ICLEntry*);
	public slots:
		void initPlugin (QObject*);

		void hookEntryActionAreasRequested (LC::IHookProxy_ptr proxy,
				QObject *action,
				QObject *entry);
		void hookEntryActionsRemoved (LC::IHookProxy_ptr proxy,
				QObject *entry);
		void hookEntryActionsRequested (LC::IHookProxy_ptr proxy,
				QObject *entryObj);
		void hookGotMessage2 (LC::IHookProxy_ptr proxy,
				QObject *message);
	private slots:
		void handlePushButton (const QString&);
	signals:
		void gotLastMessages (QObject*, const QList<QObject*>&);

		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace);
	};
}
}
}
