/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include <interfaces/core/icoreproxy.h>
#include "ui_reporttypepage.h"

namespace LC
{
namespace Dolozhee
{
	class ReportTypePage : public QWizardPage
	{
		const ICoreProxy_ptr Proxy_;

		Ui::ReportTypePage Ui_;
	public:
		enum class Type
		{
			Bug,
			Feature
		};

		enum class Priority
		{
			Low,
			Normal,
			High
		};

		explicit ReportTypePage (const ICoreProxy_ptr&, QWidget* = nullptr);

		int nextId () const override;
		void initializePage () override;

		void ForceReportType (Type);

		Type GetReportType () const;
		int GetCategoryID () const;
		QString GetCategoryName () const;
		Priority GetPriority () const;
	private:
		void ParseCategories (const QByteArray&);
	};
}
}
