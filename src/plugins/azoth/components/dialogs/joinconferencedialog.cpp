/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "joinconferencedialog.h"
#include <QSet>
#include <QPushButton>
#include <QtDebug>
#include "interfaces/azoth/iprotocol.h"
#include "interfaces/azoth/imucjoinwidget.h"
#include "interfaces/azoth/isupportbookmarks.h"
#include "interfaces/azoth/imucprotocol.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Azoth
{
	JoinConferenceDialog::JoinConferenceDialog (const QList<IAccount*>& accounts, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		setAttribute (Qt::WA_DeleteOnClose, true);

		for (const auto acc : accounts)
		{
			if (!acc->IsShownInRoster ())
				continue;

			const auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());
			const auto mucProto = qobject_cast<IMUCProtocol*> (acc->GetParentProtocol ());
			if (!mucProto)
				continue;

			if (!Proto2Joiner_.contains (proto))
			{
				const auto joiner = mucProto->GetMUCJoinWidget ();
				if (!qobject_cast<IMUCJoinWidget*> (joiner))
				{
					qWarning () << Q_FUNC_INFO
							<< "joiner widget for account"
							<< acc->GetAccountID ()
							<< "is not a IMUCJoinWidget"
							<< joiner;
					continue;
				}
				Proto2Joiner_ [proto] = joiner;
			}

			if (auto supBms = qobject_cast<ISupportBookmarks*> (acc->GetQObject ()))
				for (const auto& item : supBms->GetBookmarkedMUCs ())
				{
					const QVariantMap& map = item.toMap ();
					const QString& name = map ["HumanReadableName"].toString ();
					if (name.isEmpty ())
						continue;

					Ui_.BookmarksBox_->addItem (QString ("%1 (%2 [%3])")
								.arg (name)
								.arg (acc->GetAccountName ())
								.arg (proto->GetProtocolName ()),
							map);
				}

			Ui_.AccountBox_->addItem (tr ("%1 (%2, %3)")
						.arg (acc->GetAccountName ())
						.arg (acc->GetOurNick ())
						.arg (proto->GetProtocolName ()),
					QVariant::fromValue<QObject*> (acc->GetQObject ()));

			const auto& key = "JoinHistory/" + acc->GetAccountID ();
			for (const auto& var : XmlSettingsManager::Instance ().GetRawValue (key).toList ())
			{
				const auto& map = var.toMap ();
				const auto& name = map ["HumanReadableName"].toString ();
				if (name.isEmpty ())
					continue;

				Ui_.HistoryBox_->addItem (QString ("%1 (%2 [%3])")
							.arg (name)
							.arg (acc->GetAccountName ())
							.arg (proto->GetProtocolName ()),
						map);
			}
		}

		if (Ui_.HistoryBox_->count ())
			on_HistoryBox__activated (0);
	}

	JoinConferenceDialog::~JoinConferenceDialog ()
	{
		qDeleteAll (Proto2Joiner_);
	}

	void JoinConferenceDialog::SetIdentifyingData (const QVariantMap& ident)
	{
		FillWidget (ident);
	}

	void JoinConferenceDialog::accept ()
	{
		QDialog::accept ();

		QObject *accObj = Ui_.AccountBox_->
				itemData (Ui_.AccountBox_->currentIndex ()).value<QObject*> ();

		if (!Ui_.JoinWidgetFrameLayout_->count ())
			return;

		QWidget *widget = Ui_.JoinWidgetFrameLayout_->
				itemAt (0)->widget ();
		IMUCJoinWidget *imjw =
				qobject_cast<IMUCJoinWidget*> (widget);

		if (!imjw)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< widget
					<< "to IMUCJoinWidget";
			return;
		}

		const auto& data = imjw->GetIdentifyingData ();
		if (const auto acc = qobject_cast<IAccount*> (accObj))
		{
			const auto& key = "JoinHistory/" + acc->GetAccountID ();
			QVariantList list = XmlSettingsManager::Instance ()
					.GetRawValue (key).toList ();

			// Using a range-based for loop over a container being modified is OK
			// as the loop is terminated once the container is modified.
			for (const auto& var : list)
				if (var.toMap () ["HumanReadableName"] == data ["HumanReadableName"])
				{
					list.removeAll (var);
					break;
				}

			list.prepend (QVariant (data));
			XmlSettingsManager::Instance ().SetRawValue (key, list);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "could not cast"
					<< accObj
					<< "to IAccount";

		imjw->Join (accObj);

		auto isb = qobject_cast<ISupportBookmarks*> (accObj);
		if (Ui_.AddToBookmarks_->checkState () != Qt::Checked || !isb)
			return;

		auto list = isb->GetBookmarkedMUCs ();
		list << data;
		isb->SetBookmarkedMUCs (list);
	}

	void JoinConferenceDialog::reject ()
	{
		QDialog::reject ();

		if (Ui_.JoinWidgetFrameLayout_->count ())
		{
			QWidget *widget = Ui_.JoinWidgetFrameLayout_->
					itemAt (0)->widget ();
			IMUCJoinWidget *imjw =
					qobject_cast<IMUCJoinWidget*> (widget);

			if (!imjw)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast"
						<< widget
						<< "to IMUCJoinWidget";
				return;
			}
			imjw->Cancel ();
		}
	}

	void JoinConferenceDialog::on_AccountBox__currentIndexChanged (int idx)
	{
		while (Ui_.JoinWidgetFrameLayout_->count ())
		{
			QLayoutItem *item = Ui_.JoinWidgetFrameLayout_->takeAt (0);
			disconnect (item->widget (),
					SIGNAL (validityChanged (bool)),
					this,
					SLOT (handleValidityChanged (bool)));
			item->widget ()->hide ();
		}

		QObject *accObj = Ui_.AccountBox_->itemData (idx).value<QObject*> ();
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "item at idx"
					<< idx
					<< "doesn't yield a valid IAccount:"
					<< Ui_.AccountBox_->itemData (idx);
			return;
		}

		const auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());
		if (!proto)
		{
			qWarning () << Q_FUNC_INFO
					<< accObj
					<< "returns an invalid IProtocol"
					<< acc->GetParentProtocol ();
			return;
		}

		QWidget *joiner = Proto2Joiner_ [proto];
		Ui_.JoinWidgetFrameLayout_->addWidget (joiner);
		joiner->show ();
		connect (joiner,
				SIGNAL (validityChanged (bool)),
				this,
				SLOT (handleValidityChanged (bool)));

		adjustSize ();

		qobject_cast<IMUCJoinWidget*> (joiner)->AccountSelected (accObj);
	}

	void JoinConferenceDialog::on_BookmarksBox__activated (int idx)
	{
		const QVariantMap& map = Ui_.BookmarksBox_->itemData (idx).toMap ();
		FillWidget (map);
	}

	void JoinConferenceDialog::on_HistoryBox__activated (int idx)
	{
		const QVariantMap& map = Ui_.HistoryBox_->itemData (idx).toMap ();
		FillWidget (map);
	}

	void JoinConferenceDialog::handleValidityChanged (bool isValid)
	{
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (isValid);
	}

	void JoinConferenceDialog::FillWidget (const QVariantMap& map)
	{
		const QByteArray& id = map ["AccountID"].toByteArray ();

		bool accFound = false;
		for (int i = 0; i < Ui_.AccountBox_->count (); ++i)
		{
			QObject *accObj = Ui_.AccountBox_->itemData (i).value<QObject*> ();
			IAccount *acc = qobject_cast<IAccount*> (accObj);
			if (acc->GetAccountID () != id)
				continue;

			Ui_.AccountBox_->setCurrentIndex (i);

			const auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());
			if (!Proto2Joiner_.contains (proto))
			{
				qWarning () << Q_FUNC_INFO
						<< "Proto2Joiner_ doesn't contain protocol for account"
						<< accObj;
				return;
			}
			qobject_cast<IMUCJoinWidget*> (Proto2Joiner_ [proto])->
					SetIdentifyingData (map);

			accFound = true;
			break;
		}
		if (!accFound)
		{
			qWarning () << Q_FUNC_INFO
					<< "could not find account with ID"
					<< id
					<< "; full map follows:"
					<< map;
			return;
		}
	}
}
}
