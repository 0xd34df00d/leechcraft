/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_JOINGROUPCHATWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_JOINGROUPCHATWIDGET_H
#include <QDialog>
#include <interfaces/azoth/imucjoinwidget.h>
#include "ui_joingroupchatwidget.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class GlooxAccount;

	class JoinGroupchatWidget : public QWidget
							  , public IMUCJoinWidget
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMUCJoinWidget)

		Ui::JoinGroupchatWidget Ui_;
		GlooxAccount *SelectedAccount_;
	public:
		JoinGroupchatWidget (QWidget* = 0);

		QString GetServer () const;
		QString GetRoom () const;
		QString GetNickname () const;
		QString GetPassword () const;

		void AccountSelected (QObject *account);
		void Join (QObject *account);
		void Cancel ();

		QVariantMap GetIdentifyingData () const;
		void SetIdentifyingData (const QVariantMap& data);
	private slots:
		void checkValidity ();
		void on_ViewRooms__released ();
		void on_Server__textChanged (const QString&);
	signals:
		void validityChanged (bool);
	};
}
}
}

#endif
