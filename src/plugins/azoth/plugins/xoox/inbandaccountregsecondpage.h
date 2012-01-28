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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_INBANDACCOUNTREGSECONDPAGE_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_INBANDACCOUNTREGSECONDPAGE_H
#include <QWizardPage>
#include <QXmppClient.h>
#include "legacyformbuilder.h"
#include "formbuilder.h"

class QXmppBobManager;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class InBandAccountRegFirstPage;

	class InBandAccountRegSecondPage : public QWizardPage
	{
		Q_OBJECT

		QXmppClient *Client_;
		QXmppBobManager *BobManager_;
		InBandAccountRegFirstPage *FirstPage_;
		LegacyFormBuilder LFB_;
		FormBuilder FB_;
		QWidget *Widget_;

		enum FormType
		{
			FTLegacy,
			FTNew
		} FormType_;

		enum State
		{
			SError,
			SIdle,
			SConnecting,
			SFetchingForm,
			SAwaitingUserInput,
			SAwaitingRegistrationResult
		} State_;
	public:
		InBandAccountRegSecondPage (InBandAccountRegFirstPage*, QWidget* = 0);

		void Register ();

		QString GetJID () const;
		QString GetPassword () const;

		bool isComplete () const;
		void initializePage ();
	private:
		void ShowMessage (const QString&);
		void SetState (State);
		void HandleRegForm (const QXmppIq&);
		void HandleRegResult (const QXmppIq&);
	private slots:
		void handleConnected ();
		void handleError (QXmppClient::Error);
		void handleIqReceived (const QXmppIq&);
	signals:
		void successfulReg ();
		void regError (const QString&);
	};
}
}
}

#endif
