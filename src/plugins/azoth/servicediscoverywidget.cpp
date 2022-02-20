/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "servicediscoverywidget.h"
#include <QToolBar>
#include <QComboBox>
#include <QLineEdit>
#include <QTimer>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <util/gui/clearlineeditaddon.h>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/ihaveservicediscovery.h"
#include "core.h"

namespace LC
{
namespace Azoth
{
	QObject* ServiceDiscoveryWidget::S_ParentMultiTabs_ = 0;

	void ServiceDiscoveryWidget::SetParentMultiTabs (QObject *parent)
	{
		S_ParentMultiTabs_ = parent;
	}

	namespace
	{
		class SDFilterModel : public QSortFilterProxyModel
		{
		public:
			SDFilterModel (QObject *parent)
			: QSortFilterProxyModel (parent)
			{
				setDynamicSortFilter (true);
			}
		protected:
			bool filterAcceptsRow (int row, const QModelIndex& parent) const
			{
				const auto& filter = filterRegExp ().pattern ();
				if (filter.isEmpty ())
					return true;

				const auto& idx = sourceModel ()->index (row, 0, parent);
				for (int i = 0, rc = sourceModel ()->rowCount (idx); i < rc; ++i)
					if (filterAcceptsRow (i, idx))
						return true;

				for (int i = 0, cc = sourceModel ()->columnCount (parent); i < cc; ++i)
				{
					const auto& idx = sourceModel ()->index (row, i, parent);
					if (idx.data ().toString ().contains (filter, Qt::CaseInsensitive))
						return true;
				}

				return false;
			}
		};
	}

	ServiceDiscoveryWidget::ServiceDiscoveryWidget (QWidget *parent)
	: QWidget (parent)
	, Toolbar_ (new QToolBar)
	, AccountBox_ (new QComboBox)
	, AddressLine_ (new QLineEdit)
	, FilterLine_ (new QLineEdit)
	, FilterModel_ (new SDFilterModel (this))
	, DiscoveryTimer_ (new QTimer (this))
	{
		Ui_.setupUi (this);

		DiscoveryTimer_->setSingleShot (true);
		DiscoveryTimer_->setInterval (1500);

		new Util::ClearLineEditAddon (Core::Instance ().GetProxy (), FilterLine_);

		Toolbar_->addWidget (AccountBox_);
		Toolbar_->addWidget (AddressLine_);
		Toolbar_->addWidget (FilterLine_);
		FilterLine_->setPlaceholderText (tr ("Filter..."));

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
		connect (FilterLine_,
				SIGNAL (textChanged (QString)),
				FilterModel_,
				SLOT (setFilterFixedString (QString)));
		connect (DiscoveryTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (discover ()));

		Ui_.DiscoveryTree_->setModel (FilterModel_);

		for (const auto acc : Core::Instance ().GetAccounts ())
		{
			auto ihsd = qobject_cast<IHaveServiceDiscovery*> (acc->GetQObject ());
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
					QVariant::fromValue<QObject*> (acc->GetQObject ()));
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
			QIcon ("lcicons:/plugins/azoth/resources/images/sdtab.svg"),
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
		emit removeTab ();
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
		auto ihsd = qobject_cast<IHaveServiceDiscovery*> (obj);
		const auto& query = ihsd->GetDefaultQuery ();
		if (query.isEmpty ())
			return;

		AddressLine_->setText (query);
		discover ();
	}

	void ServiceDiscoveryWidget::SetSDSession (ISDSession *session)
	{
		AddressLine_->setText (session->GetQuery ());
		SDSession_.reset (session);

		FilterModel_->setSourceModel (session->GetRepresentationModel ());
	}

	void ServiceDiscoveryWidget::handleDiscoveryAddressChanged ()
	{
		DiscoveryTimer_->stop ();
		DiscoveryTimer_->start ();
	}

	void ServiceDiscoveryWidget::on_DiscoveryTree__customContextMenuRequested (const QPoint& point)
	{
		const auto& idx = FilterModel_->mapToSource (Ui_.DiscoveryTree_->indexAt (point));
		if (!idx.isValid ())
			return;

		const auto& actions = SDSession_->GetActionsFor (idx);
		if (actions.isEmpty ())
			return;

		QMenu *menu = new QMenu (tr ("Discovery actions"));
		for (auto i = actions.begin (), end = actions.end (); i != end; ++i)
			 menu->addAction (i->second)->setProperty ("Azoth/ID", i->first);

		QAction *result = menu->exec (Ui_.DiscoveryTree_->viewport ()->mapToGlobal (point));
		menu->deleteLater ();
		if (!result)
			return;

		const QByteArray& id = result->property ("Azoth/ID").toByteArray ();
		SDSession_->ExecuteAction (idx, id);
	}

	void ServiceDiscoveryWidget::discover ()
	{
		DiscoveryTimer_->stop ();

		FilterModel_->setSourceModel (0);
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
