/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QXmppClient.h>
#include "legacyformbuilder.h"
#include "formbuilder.h"

class QXmppClient;

namespace LeechCraft
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
