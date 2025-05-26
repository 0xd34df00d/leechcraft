/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include <util/threads/coro/taskfwd.h>
#include <interfaces/core/icoreproxy.h>
#include "ui_finalpage.h"
#include "structures.h"

namespace LC
{
struct Entity;

namespace Dolozhee
{
	class FinalPage : public QWizardPage
	{
		Q_OBJECT

		Ui::FinalPage Ui_;

		const ICoreProxy_ptr Proxy_;
	public:
		explicit FinalPage (const ICoreProxy_ptr&, QWidget* = nullptr);

		void initializePage () override;
	private:
		Util::ContextTask<void> RunUploading ();
	private slots:
		void on_Status__linkActivated (const QString&);
	};
}
}
