/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "subscriptionsmanager.h"
#include "core.h"
#include "ui_subscriptionadddialog.h"
#include <memory>
#include <QMessageBox>

using namespace LeechCraft::Plugins::Poshuku::Plugins::CleanWeb;

SubscriptionsManager::SubscriptionsManager (QWidget *parent)
: QWidget (parent)
{
	Ui_.setupUi (this);
	Ui_.Subscriptions_->setModel (Core::Instance ().GetModel ());
}

void SubscriptionsManager::on_RemoveButton__released ()
{
	QModelIndex current = Ui_.Subscriptions_->currentIndex ();
	if (!current.isValid ())
		return;

	Core::Instance ().Remove (current);
}

void SubscriptionsManager::on_AddButton__released ()
{
	Ui::SubscriptionAddDialog subscriptionAdd;
	std::auto_ptr<QDialog> subscriptionAddWidget (new QDialog (this));
	subscriptionAdd.setupUi (subscriptionAddWidget.get ());

	if (!subscriptionAddWidget->exec ())
		return;

	QUrl url (subscriptionAdd.URLEdit_->text ());
	QUrl locationUrl;
	if (url.queryItemValue ("location").contains ("%"))
		locationUrl.setEncodedUrl (url.queryItemValue ("location").toAscii ());
	else
		locationUrl.setUrl (url.queryItemValue ("location"));

	if (url.scheme () == "abp" &&
			url.host () == "subscribe" &&
			locationUrl.isValid ())
	{
		if (subscriptionAdd.TitleEdit_->text ().isEmpty ())
		{
			QMessageBox::warning (this, tr ("Error adding subscription"),
					      tr ("Can't add subscription without title"),
					      QMessageBox::Ok);
			return;
		}

		if (Core::Instance ().Exists (subscriptionAdd.TitleEdit_->text ()))
		{
			QMessageBox::warning (this, tr ("Error adding subscription"),
					      tr ("Subscription with such title allready exists"),
					      QMessageBox::Ok);
			return;
		}

		if (Core::Instance ().Exists (locationUrl))
		{
			QMessageBox::warning (this, tr ("Error adding subscription"),
					      tr ("Subscription with such title allready exists"),
					      QMessageBox::Ok);
			return;
		}
	}
	else
	{
		QMessageBox::warning (this, tr ("Error adding subscription"),
					    tr ("Invalid URL<br />Valid url format is: abp://subscribe/?location=URL"),
					    QMessageBox::Ok);
		return;
	}

	Core::Instance ().Load (locationUrl, subscriptionAdd.TitleEdit_->text ());
}

