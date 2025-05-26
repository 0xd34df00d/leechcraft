/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "reporttypepage.h"
#include <memory>
#include <QDomDocument>
#include <QtDebug>
#include <util/sll/unreachable.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <util/xpc/downloadhelpers.h>
#include "reportwizard.h"

namespace LC
{
namespace Dolozhee
{
	ReportTypePage::ReportTypePage (const ICoreProxy_ptr& proxy, QWidget *parent)
	: QWizardPage { parent }
	, Proxy_ { proxy }
	{
		Ui_.setupUi (this);
		Ui_.CatCombo_->addItem (QString ());
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

		Util::Unreachable ();
	}

	void ReportTypePage::initializePage ()
	{
		QWizardPage::initializePage ();
		if (Ui_.CatCombo_->count () > 1)
			return;

		[this] -> Util::ContextTask<void>
		{
			co_await Util::AddContextObject { *this };

			const QUrl url { "https://dev.leechcraft.org/projects/leechcraft.xml?include=issue_categories"_qs };
			const auto result = co_await Util::DownloadAsTemporary (*Proxy_->GetEntityManager (), url);
			const auto data = co_await WithHandler (result,
					[] (const IDownload::Error&) { /* TODO */ });
			ParseCategories (data);
		} ();
	}

	void ReportTypePage::ForceReportType (Type type)
	{
		switch (type)
		{
		case Type::Feature:
			Ui_.TypeCombo_->setCurrentIndex (1);
			break;
		case Type::Bug:
			Ui_.TypeCombo_->setCurrentIndex (0);
			break;
		}

		Ui_.TypeCombo_->setEnabled (false);
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

	QString ReportTypePage::GetCategoryName () const
	{
		return Ui_.CatCombo_->currentText ();
	}

	ReportTypePage::Priority ReportTypePage::GetPriority () const
	{
		return static_cast<Priority> (Ui_.PriorityBox_->currentIndex ());
	}

	void ReportTypePage::ParseCategories (const QByteArray& data)
	{
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid data"
					<< data;
			return;
		}

		auto category = doc.documentElement ()
				.firstChildElement ("issue_categories")
				.firstChildElement ("issue_category");
		while (!category.isNull ())
		{
			std::shared_ptr<void> guard (static_cast<void*> (0),
					[&category] (void*) { category = category.nextSiblingElement ("issue_category"); });
			const auto& idText = category.attribute ("id");
			bool ok = false;
			const int id = idText.toInt (&ok);
			if (!ok)
			{
				qWarning () << Q_FUNC_INFO
						<< "invalid category id"
						<< idText;
				continue;
			}

			const auto& name = category.attribute ("name");

			Ui_.CatCombo_->addItem (name, id);
		}
	}
}
}
