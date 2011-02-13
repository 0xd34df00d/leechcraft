/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#include "joinconferencedialog.h"
#include <QSet>
#include <QtDebug>
#include "interfaces/iprotocol.h"
#include "interfaces/imucjoinwidget.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	JoinConferenceDialog::JoinConferenceDialog (const QList<IAccount*>& accounts, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		Q_FOREACH (IAccount *acc, accounts)
		{
			IProtocol *proto =
					qobject_cast<IProtocol*> (acc->GetParentProtocol ());

			QWidget *joiner = 0;
			if (!Proto2Joiner_.contains (proto))
			{
				joiner = proto->GetMUCJoinWidget ();
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
			else
				joiner = Proto2Joiner_ [proto];

			IMUCJoinWidget *imjw = qobject_cast<IMUCJoinWidget*> (joiner);

			Q_FOREACH (const QVariant& item, imjw->GetBookmarkedMUCs ())
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
					QVariant::fromValue<QObject*> (acc->GetObject ()));

			const QString& key = "JoinHistory/" + acc->GetAccountID ();
			QVariantList list = XmlSettingsManager::Instance ()
					.GetRawValue (key).toList ();

			Q_FOREACH (const QVariant& var, list)
			{
				const QVariantMap& map = var.toMap ();
				const QString& name = map ["HumanReadableName"].toString ();
				if (name.isEmpty ())
					continue;

				Ui_.HistoryBox_->addItem (QString ("%1 (%2 [%3])")
							.arg (name)
							.arg (acc->GetAccountName ())
							.arg (proto->GetProtocolName ()),
						map);
			}
		}
	}

	JoinConferenceDialog::~JoinConferenceDialog ()
	{
		qDeleteAll (Proto2Joiner_.values ());
	}

	void JoinConferenceDialog::accept ()
	{
		QDialog::accept ();

		QObject *accObj = Ui_.AccountBox_->
				itemData (Ui_.AccountBox_->currentIndex ()).value<QObject*> ();

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

			const QVariantMap& data = imjw->GetIdentifyingData ();
			IAccount *acc = qobject_cast<IAccount*> (accObj);
			if (acc)
			{
				const QString& key = "JoinHistory/" + acc->GetAccountID ();
				QVariantList list = XmlSettingsManager::Instance ()
						.GetRawValue (key).toList ();

				bool found = false;
				Q_FOREACH (const QVariant& var, list)
					if (var.toMap () ["HumanReadableName"] == data ["HumanReadableName"])
					{
						found = true;
						break;
					}

				if (!found)
				{
					list << QVariant (data);
					XmlSettingsManager::Instance ().SetRawValue (key, list);
				}
			}
			else
				qWarning () << Q_FUNC_INFO
						<< "could not cast"
						<< accObj
						<< "to IAccount";

			imjw->Join (accObj);
		}
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
			Ui_.JoinWidgetFrameLayout_->removeItem (Ui_.JoinWidgetFrameLayout_->itemAt (0));

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

		IProtocol *proto =
				qobject_cast<IProtocol*> (acc->GetParentProtocol ());
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

		qobject_cast<IMUCJoinWidget*> (joiner)->AccountSelected (accObj);
	}

	/** @todo Implement the support for bookmarks. It'd be
		* easier to test this after we have a working plugin with
		* support for bookmarks.
		*/
	void JoinConferenceDialog::on_BookmarksBox__currentIndexChanged (int)
	{
	}

	void JoinConferenceDialog::on_HistoryBox__currentIndexChanged (int idx)
	{
		const QVariantMap& map = Ui_.HistoryBox_->itemData (idx).toMap ();
		const QByteArray& id = map ["AccountID"].toByteArray ();

		bool accFound = false;
		for (int i = 0; i < Ui_.AccountBox_->count (); ++i)
		{
			QObject *accObj = Ui_.AccountBox_->itemData (i).value<QObject*> ();
			IAccount *acc = qobject_cast<IAccount*> (accObj);
			if (acc->GetAccountID () != id)
				continue;

			Ui_.AccountBox_->setCurrentIndex (i);

			IProtocol *proto =
					qobject_cast<IProtocol*> (acc->GetParentProtocol ());
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
