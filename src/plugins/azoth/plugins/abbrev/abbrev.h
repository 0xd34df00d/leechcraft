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
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/azoth/iprovidecommands.h>

class QWebEngineView;

namespace LC::Azoth
{
class IProxyObject;

namespace Abbrev
{
	class AbbrevsManager;
	class ShortcutsManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveShortcuts
				 , public IProvideCommands
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				IHaveShortcuts
				LC::Azoth::IProvideCommands)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.Abbrev")

		StaticCommands_t Commands_;

		IProxyObject *AzothProxy_ = nullptr;

		std::shared_ptr<AbbrevsManager> Manager_;

		ShortcutsManager *ShortcutsMgr_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		QMap<QString, ActionInfo> GetActionInfo () const override;
		void SetShortcut (const QString&, const QKeySequences_t&) override;

		StaticCommands_t GetStaticCommands (ICLEntry*) override;
	private:
		void ListAbbrevs (ICLEntry*);
		void RemoveAbbrev (const QString&);
	public slots:
		void initPlugin (QObject*);

		void hookChatTabCreated (LC::IHookProxy_ptr,
				QObject*, QObject*, QWebEngineView*);
		void hookMessageSendRequested (LC::IHookProxy_ptr,
				QObject*,
				QObject*,
				int,
				QString);
	};
}
}

