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

#include "reporttypepage.h"
#include <memory>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDomDocument>
#include <QtDebug>
#include "reportwizard.h"

namespace LeechCraft
{
namespace Dolozhee
{
	ReportTypePage::ReportTypePage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);
	}

	int ReportTypePage::nextId () const
	{
		switch (GetReportType ())
		{
		case Type::Feature:
			return ReportWizard::PageID::FeatureDetails;
		case Type::Bug:
			return ReportWizard::PageID::BugDetails;
		}

		qWarning () << Q_FUNC_INFO
				<< "invalid report type";
		return -1;
	}

	void ReportTypePage::initializePage ()
	{
		QWizardPage::initializePage ();
		if (Ui_.CatCombo_->count ())
			return;

		auto rw = static_cast<ReportWizard*> (wizard ());
		QNetworkRequest req (QUrl ("http://dev.leechcraft.org/projects/leechcraft/issue_categories.xml"));
		req.setHeader (QNetworkRequest::ContentTypeHeader, "application/xml");
		auto reply = rw->GetNAM ()->get (req);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleCategoriesFinished ()));
	}

	ReportTypePage::Type ReportTypePage::GetReportType () const
	{
		return Ui_.TypeCombo_->currentIndex () == 1 ? Type::Feature : Type::Bug;
	}

	int ReportTypePage::GetCategoryID () const
	{
		const int idx = Ui_.CatCombo_->currentIndex ();
		return idx > 0 ?
				Ui_.CatCombo_->itemData (idx).toInt () :
				-1;
	}

	void ReportTypePage::handleCategoriesFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid reply"
					<< sender ();
			return;
		}

		reply->deleteLater ();

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid data"
					<< data;
			return;
		}

		auto category = doc.documentElement ().firstChildElement ("issue_category");
		while (!category.isNull ())
		{
			std::shared_ptr<void> guard (static_cast<void*> (0),
					[&category] (void*) { category = category.nextSiblingElement ("issue_category"); });
			const auto& idText = category.firstChildElement ("id").text ();
			bool ok = false;
			const int id = idText.toInt (&ok);
			if (!ok)
			{
				qWarning () << Q_FUNC_INFO
						<< "invalid category id"
						<< idText;
				continue;
			}

			const auto& name = category.firstChildElement ("name").text ();

			Ui_.CatCombo_->addItem (name, id);
		}
	}
}
}
