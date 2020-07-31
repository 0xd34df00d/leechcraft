/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include <QMap>
#include "ui_firstpage.h"

namespace LC
{
namespace NewLife
{
	class AbstractImporter;

	class FirstPage : public QWizardPage
	{
		Q_OBJECT

		Ui::FirstPage Ui_;
		QMap<const AbstractImporter*, int> StartPages_;
	public:
		FirstPage (QWidget* = 0);

		virtual int nextId () const;

		void SetupImporter (AbstractImporter*);
		AbstractImporter* GetImporter () const;
		QString GetSelectedName () const;
	};
}
}
