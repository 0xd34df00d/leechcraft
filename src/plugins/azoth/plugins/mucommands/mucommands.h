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
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/azoth/iprovidecommands.h>

namespace LC
{
namespace Azoth
{
class IProxyObject;

namespace MuCommands
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IProvideCommands
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				LC::Azoth::IProvideCommands)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.MuCommands")

		StaticCommand Names_;
		StaticCommand ListUrls_;
		StaticCommand OpenUrl_;
		StaticCommand FetchUrl_;
		StaticCommand VCard_;
		StaticCommand Version_;
		StaticCommand Time_;
		StaticCommand Disco_;
		StaticCommand ChangeNick_;
		StaticCommand ChangeSubject_;
		StaticCommand JoinMuc_;
		StaticCommand LeaveMuc_;
		StaticCommand RejoinMuc_;
		StaticCommand Ping_;
		StaticCommand Last_;
		StaticCommand Invite_;
		StaticCommand Pm_;
		StaticCommand Whois_;
		StaticCommand ListPerms_;
		StaticCommand SetPerm_;
		StaticCommand Kick_;
		StaticCommand Ban_;
		StaticCommand Subst_;
		StaticCommand Presence_;
		StaticCommand ChatPresence_;

		ICoreProxy_ptr CoreProxy_;
		IProxyObject *AzothProxy_ = nullptr;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		StaticCommands_t GetStaticCommands (ICLEntry*);
	public slots:
		void initPlugin (QObject*);
	};
}
}
}
