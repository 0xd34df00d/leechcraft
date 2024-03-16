/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QAction>
#include <QObject>
#include <QIcon>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>

class QTranslator;
class QAction;

namespace LC::Azoth::Depester
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.Depester")

		QHash<QObject*, QAction*> Entry2ActionIgnore_;
		QHash<QObject*, QString> Entry2Nick_;
		QSet<QString> IgnoredNicks_;

		QAction IgnoredAction_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;
	private:
		bool IsEntryIgnored (QObject*);
		void HandleMsgOccurence (IHookProxy_ptr, QObject*);
		void SaveIgnores () const;
		void LoadIgnores ();
	public slots:
		void hookEntryActionAreasRequested (LC::IHookProxy_ptr proxy,
				QObject *action,
				QObject *entry);
		void hookEntryActionsRemoved (LC::IHookProxy_ptr proxy,
				QObject *entry);
		void hookEntryActionsRequested (LC::IHookProxy_ptr proxy,
				QObject *entry);
		void hookGonnaAppendMsg (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookGotMessage (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookShouldCountUnread (LC::IHookProxy_ptr proxy,
				QObject *message);

		void hookCollectContactIcons (LC::IHookProxy_ptr, QObject*, QList<QIcon>&);
	private slots:
		void handleIgnoreEntry (bool);
		void handleNameChanged (const QString&);
	};
}
