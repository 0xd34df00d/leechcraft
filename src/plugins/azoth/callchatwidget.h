/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_CALLCHATWIDGET_H
#define PLUGINS_AZOTH_CALLCHATWIDGET_H
#include <QWidget>
#include "interfaces/azoth/imediacall.h"
#include "ui_callchatwidget.h"

namespace LC
{
namespace Azoth
{
	class CallChatWidget : public QWidget
	{
		Q_OBJECT

		Ui::CallChatWidget Ui_;
		QObject * const CallObject_;
		IMediaCall * const Call_;
	public:
		CallChatWidget (QObject*, QWidget* = 0);
	private slots:
		void on_AcceptButton__released ();
		void on_HangupButton__released ();
		void handleStateChanged (LC::Azoth::IMediaCall::State);
		void scheduleDelete ();
	};
}
}

#endif
