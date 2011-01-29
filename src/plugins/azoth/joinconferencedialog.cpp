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

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			JoinConferenceDialog::JoinConferenceDialog (const QList<Plugins::IAccount*>& accounts, QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);

				Q_FOREACH (Plugins::IAccount *acc, accounts)
				{
					Plugins::IProtocol *proto =
							qobject_cast<Plugins::IProtocol*> (acc->GetParentProtocol ());
					if (Proto2Joiner_.contains (proto))
						continue;

					QWidget *joiner = proto->GetMUCJoinWidget ();
					Proto2Joiner_ [proto] = joiner;

					Plugins::IMUCJoinWidget *imjw = qobject_cast<Plugins::IMUCJoinWidget*> (joiner);

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

					Ui_.AccountBox_->addItem (tr ("%1 (nick %2, protocol %3)")
								.arg (acc->GetAccountName ())
								.arg (acc->GetOurNick ())
								.arg (proto->GetProtocolName ()),
							QVariant::fromValue<QObject*> (acc->GetObject ()));
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
					Plugins::IMUCJoinWidget *imjw =
							qobject_cast<Plugins::IMUCJoinWidget*> (widget);

					if (!imjw)
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to cast"
								<< widget
								<< "to IMUCJoinWidget";
						return;
					}
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
					Plugins::IMUCJoinWidget *imjw =
							qobject_cast<Plugins::IMUCJoinWidget*> (widget);

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
				Plugins::IAccount *acc = qobject_cast<Plugins::IAccount*> (accObj);
				if (!acc)
				{
					qWarning () << Q_FUNC_INFO
							<< "item at idx"
							<< idx
							<< "doesn't yield a valid IAccount:"
							<< Ui_.AccountBox_->itemData (idx);
					return;
				}

				Plugins::IProtocol *proto =
						qobject_cast<Plugins::IProtocol*> (acc->GetParentProtocol ());
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

				qobject_cast<Plugins::IMUCJoinWidget*> (joiner)->AccountSelected (accObj);
			}
		}
	}
}
