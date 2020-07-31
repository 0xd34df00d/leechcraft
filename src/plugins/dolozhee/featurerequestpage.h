/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include "ui_featurerequestpage.h"

namespace LC
{
namespace Dolozhee
{
	class FeatureRequestPage : public QWizardPage
	{
		Ui::FeatureRequestPage Ui_;
	public:
		explicit FeatureRequestPage (QWidget* = nullptr);

		int nextId () const override;
		bool isComplete () const override;

		QString GetTitle () const;
		QString GetText () const;
		QList<QPair<QString, QString>> GetReportSections () const;
	};
}
}
