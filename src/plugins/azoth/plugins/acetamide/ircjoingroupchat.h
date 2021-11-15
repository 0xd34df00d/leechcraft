/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/azoth/imucjoinwidget.h>
#include "ui_ircjoingroupchat.h"
#include "localtypes.h"

namespace LC::Azoth::Acetamide
{
	class IrcAccount;

	class IrcJoinGroupChat : public QWidget
						   , public IMUCJoinWidget
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMUCJoinWidget)

		Ui::IrcJoinGroupChat Ui_;
		IrcAccount *SelectedAccount_ = nullptr;
	public:
		explicit IrcJoinGroupChat (QWidget* = nullptr);

		void AccountSelected (QObject*) override;
		void Join (QObject*) override;
		void Cancel () override;
		void SetIdentifyingData (const QVariantMap&) override;
		QVariantMap GetIdentifyingData () const override;

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
		void validityChanged (bool) override;
	};
}
