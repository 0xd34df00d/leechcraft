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
#include <interfaces/ihavesettings.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/azoth/iaccountactionsprovider.h>

namespace LC
{
namespace Azoth
{
class IMessage;

namespace Herbicide
{
	class Logger;
	class ListsHolder;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
				 , public IAccountActionsProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings LC::Azoth::IAccountActionsProvider)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.Herbicide")

		Util::XmlSettingsDialog_ptr SettingsDialog_;

		Logger *Logger_ = nullptr;

		QSet<QObject*> AskedEntries_;
		QSet<QObject*> AllowedEntries_;
		QSet<IMessage*> OurMessages_;

		QHash<QObject*, QString> DeniedAuth_;

		std::shared_ptr<ListsHolder> ListsHolder_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QList<QAction*> CreateActions (IAccount*);
	private:
		bool IsConfValid (IAccount*) const;
		bool IsEntryAllowed (QObject*) const;

		void ChallengeEntry (IHookProxy_ptr, QObject*);
		void GreetEntry (QObject*);

		void ShowAccountAntispamConfig (IAccount*);
	public slots:
		void hookGotAuthRequest (LC::IHookProxy_ptr proxy,
				QObject *entry,
				QString msg);
		void hookGotMessage (LC::IHookProxy_ptr proxy,
				QObject *message);
	};
}
}
}
