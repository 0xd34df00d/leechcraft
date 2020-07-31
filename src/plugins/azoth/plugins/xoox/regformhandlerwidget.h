/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QXmppClient.h>
#include "legacyformbuilder.h"
#include "formbuilder.h"

class QXmppClient;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class XMPPBobManager;

	class RegFormHandlerWidget : public QWidget
	{
		Q_OBJECT

		QXmppClient *Client_;
		XMPPBobManager *BobManager_;
		LegacyFormBuilder LFB_;
		FormBuilder FB_;
		QWidget *Widget_;

		QString LastStanzaID_;

		QString ReqJID_;

		enum FormType
		{
			FTLegacy,
			FTNew
		} FormType_;
	public:
		enum class State
		{
			Error,
			Idle,
			Connecting,
			FetchingForm,
			AwaitingUserInput,
			AwaitingRegistrationResult
		};
	private:
		State State_;
	public:
		RegFormHandlerWidget (QXmppClient*, QWidget* = 0);

		QString GetUser () const;
		QString GetPassword () const;

		bool IsComplete () const;

		void HandleConnecting (const QString&);

		void SendRequest (const QString& jid = QString ());
		void Register ();
	private:
		void Clear ();
		void ShowMessage (const QString&);
		void SetState (State);
		void HandleRegForm (const QXmppIq&);
		void HandleRegResult (const QXmppIq&);
	private slots:
		void handleError (QXmppClient::Error);
		void handleIqReceived (const QXmppIq&);
	signals:
		void completeChanged ();
		void successfulReg ();
		void regError (const QString&);
	};
}
}
}
