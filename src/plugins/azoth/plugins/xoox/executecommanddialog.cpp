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

#include "executecommanddialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include "glooxaccount.h"
#include "clientconnection.h"
#include "ui_commandslistpage.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class WaitPage : public QWizardPage
	{
		bool DataFetched_;
	public:
		WaitPage (const QString& text, QWidget *parent = 0)
		: QWizardPage (parent)
		, DataFetched_ (false)
		{
			setTitle (tr ("Fetching data..."));
			setCommitPage (true);
			
			setLayout (new QVBoxLayout ());
			layout ()->addWidget (new QLabel (text));
		}
		
		bool isComplete () const
		{
			return DataFetched_;
		}
		
		void SetDataFetched ()
		{
			DataFetched_ = true;
			emit completeChanged ();
		}
	};
	
	class CommandsListPage : public QWizardPage
	{
		Ui::CommandsListPage Ui_;
		QList<AdHocCommand> Commands_;
	public:
		CommandsListPage (const QList<AdHocCommand>& commands, QWidget *parent = 0)
		: QWizardPage (parent)
		, Commands_ (commands)
		{
			Ui_.setupUi (this);
			setCommitPage (true);

			Q_FOREACH (const AdHocCommand& cmd, commands)
				Ui_.CommandsBox_->addItem (cmd.GetName ());
			
			registerField ("Command", Ui_.CommandsBox_);
		}
	};

	ExecuteCommandDialog::ExecuteCommandDialog (const QString& jid,
			GlooxAccount *account, QWidget *parent)
	: QWizard (parent)
	, Account_ (account)
	, Manager_ (account->GetClientConnection ()->GetAdHocCommandManager ())
	, JID_ (jid)
	{
		Ui_.setupUi (this);
		
		RequestCommands ();
	}
	
	void ExecuteCommandDialog::RequestCommands ()
	{
		const int idx = addPage (new WaitPage (tr ("Please wait while "
				"the list of commands is fetched.")));
		if (currentId () != idx)
			next ();

		connect (Manager_,
				SIGNAL (gotCommands (QString, QList<AdHocCommand>)),
				this,
				SLOT (handleGotCommands (QString, QList<AdHocCommand>)));
		Manager_->QueryCommands (JID_);
	}
	
	void ExecuteCommandDialog::handleGotCommands (const QString& jid, const QList<AdHocCommand>& commands)
	{
		if (jid != JID_)
			return;

		disconnect (Manager_,
				SIGNAL (gotCommands (QString, QList<AdHocCommand>)),
				this,
				SLOT (handleGotCommands (QString, QList<AdHocCommand>)));
		
		addPage (new CommandsListPage (commands));
		next ();
	}
}
}
}
