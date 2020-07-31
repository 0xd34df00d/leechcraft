/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include "ui_previewpage.h"

namespace LC
{
namespace Dolozhee
{
	class PreviewPage : public QWizardPage
	{
		Ui::RequestPreview Ui_;
	public:
		explicit PreviewPage (QWidget *parent = nullptr);

		void initializePage () override;
		int nextId () const override;
	};
}
}
