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
#include "ui_bugreportpage.h"

namespace LC
{
namespace Dolozhee
{
	class BugReportPage : public QWizardPage
	{
		Ui::BugReportPage Ui_;
		ICoreProxy_ptr Proxy_;
	public:
		explicit BugReportPage (ICoreProxy_ptr, QWidget* = nullptr);

		int nextId () const override;
		bool isComplete () const override;

		QString GetTitle () const;
		QString GetText () const;
		QList<QPair<QString, QString>> GetReportSections () const;
	};
}
}
