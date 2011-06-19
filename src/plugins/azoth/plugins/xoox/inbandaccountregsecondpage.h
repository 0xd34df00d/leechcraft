/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
		InBandAccountRegFirstPage *FirstPage_;
		LegacyFormBuilder LFB_;
		FormBuilder FB_;
		QWidget *Widget_;
		
		enum State
		{
			SError,
			SIdle,
			SConnecting,
			SFetchingForm,
			SAwaitingUserInput
		} State_;
	public:
		InBandAccountRegSecondPage (InBandAccountRegFirstPage*, QWidget* = 0);

		bool isComplete () const;
		void initializePage ();
	private:
		void ShowMessage (const QString&);
		void SetState (State);
	private slots:
		void handleConnected ();
		void handleError (QXmppClient::Error);
		void handleIqReceived (const QXmppIq&);
	};
}
}
}

#endif
