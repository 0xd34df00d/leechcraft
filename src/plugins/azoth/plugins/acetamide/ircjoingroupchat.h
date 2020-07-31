/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCJOINGROUPCHAT_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCJOINGROUPCHAT_H

#include <QWidget>
#include <interfaces/azoth/imucjoinwidget.h>
#include "ui_ircjoingroupchat.h"
#include "core.h"
#include "localtypes.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	class IrcAccount;

	class IrcJoinGroupChat : public QWidget
						   , public IMUCJoinWidget
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMUCJoinWidget)

		Ui::IrcJoinGroupChat Ui_;
		IrcAccount *SelectedAccount_;
	public:
		IrcJoinGroupChat (QWidget* = 0);

		void AccountSelected (QObject*);
		void Join (QObject*);
		void Cancel ();
		void SetIdentifyingData (const QVariantMap&);
		QVariantMap GetIdentifyingData () const;

		QString GetServer () const;
		int GetPort () const;
		QString GetServerPassword () const;
		QString GetChannel () const;
		QString GetNickname () const;
		QString GetEncoding () const;
		QString GetChannelPassword () const;
		bool GetSSL () const;

		ServerOptions GetServerOptions () const;
		ChannelOptions GetChannelOptions () const;
	signals:
		void validityChanged (bool);
	};
}
}
}

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCJOINGROUPCHAT_H
