/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "subscriptionsmanagerwidget.h"
#include <QMessageBox>
#include <util/sll/urlaccessor.h>
#include "core.h"
#include "subscriptionsmodel.h"
#include "subscriptionadddialog.h"

namespace LC
{
namespace Poshuku
{
namespace CleanWeb
{
	SubscriptionsManagerWidget::SubscriptionsManagerWidget (Core *core,
			SubscriptionsModel *model, QWidget *parent)
	: QWidget { parent }
	, Core_ { core }
	, Model_ { model }
	{
		Ui_.setupUi (this);
		Ui_.Subscriptions_->setModel (model);
	}

	void SubscriptionsManagerWidget::on_RemoveButton__released ()
	{
		const auto& current = Ui_.Subscriptions_->currentIndex ();
		if (!current.isValid ())
			return;

		Model_->RemoveFilter (current);
	}

	void SubscriptionsManagerWidget::AddCustom (const QString& title, const QString& urlStr)
	{
		QUrl url (urlStr);
		QUrl locationUrl;

		const auto& location = Util::UrlAccessor { url } ["location"];
		if (location.contains ("%"))
			locationUrl.setUrl (QUrl::fromPercentEncoding (location.toLatin1 ()));
		else
			locationUrl.setUrl (location);

		if (url.scheme () != "abp" ||
				url.host () != "subscribe" ||
				!locationUrl.isValid ())
		{
			QMessageBox::warning (this,
					tr ("Error adding subscription"),
					tr ("Invalid URL. Valid URL format is %1.")
						.arg ("<em>abp://subscribe/?location=URL</em>"),
					QMessageBox::Ok);
			return;
		}

		if (title.isEmpty ())
		{
			QMessageBox::warning (this,
					tr ("Error adding subscription"),
					tr ("Can't add subscription without a title."),
					QMessageBox::Ok);
			return;
		}

		if (Model_->HasFilter (title, [] (const Filter& f) { return f.SD_.Name_; }))
		{
			QMessageBox::warning (this,
					tr ("Error adding subscription"),
					tr ("Subscription with this title already exists."),
					QMessageBox::Ok);
			return;
		}

		if (Model_->HasFilter (title, [] (const Filter& f) { return f.SD_.URL_; }))
		{
			QMessageBox::warning (this,
					tr ("Error adding subscription"),
					tr ("Subscription with this URL already exists."),
					QMessageBox::Ok);
			return;
		}

		Core_->Load (locationUrl, title);
	}

	void SubscriptionsManagerWidget::on_AddButton__released ()
	{
		SubscriptionAddDialog subscriptionAdd (this);

		if (!subscriptionAdd.exec ())
			return;

		QString title = subscriptionAdd.GetName ();
		QString urlStr = subscriptionAdd.GetURL ();
		if (!title.isEmpty () ||
				!urlStr.isEmpty ())
			AddCustom (title, urlStr);

		const auto& urls = subscriptionAdd.GetAdditionalSubscriptions ();
		for (const auto& url : urls)
			Core_->Add (url);
	}
}
}
}
