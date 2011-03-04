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
#include <memory>
#include <QMessageBox>
#include "core.h"
#include "subscriptionadddialog.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Poshuku
{
namespace Plugins
{
namespace CleanWeb
{
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

	void SubscriptionsManager::AddCustom (const QString& title, const QString& urlStr)
	{
		QUrl url (urlStr);
		QUrl locationUrl;
		if (url.queryItemValue ("location").contains ("%"))
			locationUrl.setUrl (QUrl::fromPercentEncoding (url.queryItemValue ("location").toAscii ()));
		else
			locationUrl.setUrl (url.queryItemValue ("location"));

		if (url.scheme () == "abp" &&
				url.host () == "subscribe" &&
				locationUrl.isValid ())
		{
			if (title.isEmpty ())
			{
				QMessageBox::warning (this,
						tr ("Error adding subscription"),
						tr ("Can't add subscription without a title."),
						QMessageBox::Ok);
				return;
			}

			if (Core::Instance ().Exists (title))
			{
				QMessageBox::warning (this,
						tr ("Error adding subscription"),
						tr ("Subscription with such title allready exists."),
						QMessageBox::Ok);
				return;
			}

			if (Core::Instance ().Exists (locationUrl))
			{
				QMessageBox::warning (this,
						tr ("Error adding subscription"),
						tr ("Subscription with such title allready exists."),
						QMessageBox::Ok);
				return;
			}
		}
		else
		{
			QMessageBox::warning (this,
					tr ("Error adding subscription"),
					tr ("Invalid URL.<br />Valid URL format is: abp://subscribe/?location=URL"),
					QMessageBox::Ok);
			return;
		}

		Core::Instance ().Load (locationUrl, title);
	}

	void SubscriptionsManager::on_AddButton__released ()
	{
		std::auto_ptr<SubscriptionAddDialog> subscriptionAdd (new SubscriptionAddDialog (this));

		if (!subscriptionAdd->exec ())
			return;

		QString title = subscriptionAdd->GetName ();
		QString urlStr = subscriptionAdd->GetURL ();
		if (!title.isEmpty () ||
				!urlStr.isEmpty ())
			AddCustom (title, urlStr);

		QList<QUrl> urls = subscriptionAdd->GetAdditionalSubscriptions ();
		Q_FOREACH (const QUrl& url, urls)
			Core::Instance ().Add (url);
	}
}
}
}
}
}
