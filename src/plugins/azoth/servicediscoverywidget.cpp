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

#include "servicediscoverywidget.h"
#include <QToolBar>
#include <QComboBox>
#include <QLineEdit>
#include <QTimer>
#include <QMenu>
#include "interfaces/iaccount.h"
#include "interfaces/ihaveservicediscovery.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
	QObject* ServiceDiscoveryWidget::S_ParentMultiTabs_ = 0;

	void ServiceDiscoveryWidget::SetParentMultiTabs (QObject *parent)
	{
		S_ParentMultiTabs_ = parent;
	}

	ServiceDiscoveryWidget::ServiceDiscoveryWidget (QWidget *parent)
	: QWidget (parent)
	, Toolbar_ (new QToolBar)
	, AccountBox_ (new QComboBox)
	, AddressLine_ (new QLineEdit)
	, DiscoveryTimer_ (new QTimer (this))
	{
		Ui_.setupUi (this);

		DiscoveryTimer_->setSingleShot (true);
		DiscoveryTimer_->setInterval (1500);

		Toolbar_->addWidget (AccountBox_);
		Toolbar_->addWidget (AddressLine_);

		connect (AccountBox_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (discover ()));
		connect (AddressLine_,
				SIGNAL (textEdited (const QString&)),
				this,
				SLOT (handleDiscoveryAddressChanged ()));
		connect (AddressLine_,
				SIGNAL (returnPressed ()),
				this,
				SLOT (discover ()));
		connect (DiscoveryTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (discover ()));

		Q_FOREACH (IAccount *acc, Core::Instance ().GetAccounts ())
		{
			auto ihsd = qobject_cast<IHaveServiceDiscovery*> (acc->GetObject ());
			if (!ihsd)
				continue;

			auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());
			if (!proto)
			{
				qWarning () << Q_FUNC_INFO
						<< acc->GetParentProtocol ()
						<< "doesn't implement IProtocol";
				continue;
			}

			const QString& protoName = proto->GetProtocolName ();
			AccountBox_->addItem (acc->GetAccountName () + "(" + protoName + ")",
					QVariant::fromValue<QObject*> (acc->GetObject ()));
		}
	}

	TabClassInfo ServiceDiscoveryWidget::GetTabClassInfo () const
	{
		TabClassInfo sdTab =
		{
			"SD",
			tr ("Service discovery"),
			tr ("A service discovery tab that allows one to discover "
				"capabilities of remote entries"),
			QIcon (":/plugins/azoth/resources/images/sdtab.svg"),
			55,
			TFOpenableByRequest
		};
		return sdTab;
	}

	QObject* ServiceDiscoveryWidget::ParentMultiTabs ()
	{
		return S_ParentMultiTabs_;
	}

	void ServiceDiscoveryWidget::Remove ()
	{
		emit removeTab (this);
		deleteLater ();
	}

	QToolBar* ServiceDiscoveryWidget::GetToolBar () const
	{
		return Toolbar_;
	}

	void ServiceDiscoveryWidget::SetAccount (QObject *obj)
	{
		const int pos = AccountBox_->findData (QVariant::fromValue<QObject*> (obj));
		if (pos == -1)
			return;

		AccountBox_->setCurrentIndex (pos);
	}

	void ServiceDiscoveryWidget::SetSDSession (ISDSession *session)
	{
		AddressLine_->setText (session->GetQuery ());
		SDSession_.reset (session);
		if (Ui_.DiscoveryTree_->selectionModel ())
			Ui_.DiscoveryTree_->selectionModel ()->deleteLater ();
		Ui_.DiscoveryTree_->setModel (session->GetRepresentationModel ());
	}

	void ServiceDiscoveryWidget::handleDiscoveryAddressChanged ()
	{
		DiscoveryTimer_->stop ();
		DiscoveryTimer_->start ();
	}

	void ServiceDiscoveryWidget::on_DiscoveryTree__customContextMenuRequested (const QPoint& point)
	{
		const QModelIndex& idx = Ui_.DiscoveryTree_->indexAt (point);
		if (!idx.isValid ())
			return;

		const QList<QPair<QByteArray, QString>>& actions =
				SDSession_->GetActionsFor (idx);
		if (actions.isEmpty ())
			return;

		QMenu *menu = new QMenu (tr ("Discovery actions"));
		for (auto i = actions.begin (), end = actions.end (); i != end; ++i)
			 menu->addAction (i->second)->setProperty ("Azoth/ID", i->first);

		QAction *result = menu->exec (Ui_.DiscoveryTree_->
					viewport ()->mapToGlobal (point));
		menu->deleteLater ();
		if (!result)
			return;

		const QByteArray& id = result->property ("Azoth/ID").toByteArray ();
		SDSession_->ExecuteAction (idx, id);
	}

	void ServiceDiscoveryWidget::discover ()
	{
		DiscoveryTimer_->stop ();

		if (Ui_.DiscoveryTree_->selectionModel ())
			Ui_.DiscoveryTree_->selectionModel ()->deleteLater ();
		Ui_.DiscoveryTree_->setModel (0);
		SDSession_.reset ();

		const int index = AccountBox_->currentIndex ();
		if (index == -1)
			return;

		const QString& address = AddressLine_->text ();
		if (address.isEmpty ())
			return;

		QObject *accObj = AccountBox_->itemData (index).value<QObject*> ();
		IHaveServiceDiscovery *ihsd = qobject_cast<IHaveServiceDiscovery*> (accObj);
		if (!ihsd)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< accObj
					<< "to IHaveServiceDiscovery";
			return;
		}

		QObject *sessionObj = ihsd->CreateSDSession ();
		ISDSession *session = qobject_cast<ISDSession*> (sessionObj);
		if (!session)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< sessionObj
					<< "to ISDSession; got it from"
					<< accObj;
			return;
		}

		session->SetQuery (address);
		SetSDSession (session);
	}
}
}
