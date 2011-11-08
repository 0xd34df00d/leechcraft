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

#include "mailtab.h"
#include <QToolBar>
#include "core.h"

namespace LeechCraft
{
namespace Snails
{
	MailTab::MailTab (const TabClassInfo& tc, QObject *pmt, QWidget *parent)
	: QWidget (parent)
	, TabToolbar_ (new QToolBar)
	, TabClass_ (tc)
	, PMT_ (pmt)
	{
		Ui_.setupUi (this);

		Ui_.AccountsTree_->setModel (Core::Instance ().GetAccountsModel ());

		QAction *fetch = new QAction (tr ("Fetch new mail"), this);
		TabToolbar_->addAction (fetch);
		connect (fetch,
				SIGNAL (triggered ()),
				this,
				SLOT (handleFetchNewMail ()));
	}

	TabClassInfo MailTab::GetTabClassInfo () const
	{
		return TabClass_;
	}

	QObject* MailTab::ParentMultiTabs ()
	{
		return PMT_;
	}

	void MailTab::Remove ()
	{
		emit removeTab (this);
		deleteLater ();
	}

	QToolBar* MailTab::GetToolBar () const
	{
		return TabToolbar_;
	}

	void MailTab::handleFetchNewMail ()
	{
		Q_FOREACH (auto acc, Core::Instance ().GetAccounts ())
		{
			acc->FetchNewHeaders (1);
		}
	}
}
}
