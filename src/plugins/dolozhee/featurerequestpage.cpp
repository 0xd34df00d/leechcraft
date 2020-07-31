/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "featurerequestpage.h"
#include "reportwizard.h"

namespace LC
{
namespace Dolozhee
{
	FeatureRequestPage::FeatureRequestPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);

		connect (Ui_.Title_,
				SIGNAL (textChanged (QString)),
				this,
				SIGNAL (completeChanged ()));
		connect (Ui_.Description_,
				SIGNAL (textChanged ()),
				this,
				SIGNAL (completeChanged ()));
	}

	int FeatureRequestPage::nextId () const
	{
		return ReportWizard::PageID::FilePage;
	}

	bool FeatureRequestPage::isComplete () const
	{
		return !GetTitle ().isEmpty () && !GetText ().isEmpty ();
	}

	QString FeatureRequestPage::GetTitle () const
	{
		return Ui_.Title_->text ();
	}

	QString FeatureRequestPage::GetText () const
	{
		return Ui_.Description_->toPlainText ();
	}

	QList<QPair<QString, QString>> FeatureRequestPage::GetReportSections () const
	{
		return { { "Description", Ui_.Description_->toPlainText () } };
	}
}
}
