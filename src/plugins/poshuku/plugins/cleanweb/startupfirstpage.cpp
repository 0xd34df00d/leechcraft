/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "startupfirstpage.h"
#include <typeinfo>
#include <QLineEdit>
#include <QTextCodec>
#include <QComboBox>
#include <QMessageBox>
#include <QRadioButton>
#include <util/util.h>
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LC
{
namespace Poshuku
{
namespace CleanWeb
{
	StartupFirstPage::StartupFirstPage (Core *core, QWidget *parent)
	: QWizardPage { parent }
	, Core_ { core }
	{
		Ui_.setupUi (this);
	}

	void StartupFirstPage::initializePage ()
	{
		connect (wizard (),
				SIGNAL (accepted ()),
				this,
				SLOT (handleAccepted ()));
	}

	namespace
	{
		QList<QUrl> GetChildUrls (QWidget *w)
		{
			QList<QUrl> result;
			for (const auto cb : w->findChildren<QCheckBox*> ())
				if (cb->isChecked ())
					result << cb->property ("ListURL").value<QUrl> ();

			for (const auto but : w->findChildren<QRadioButton*> ())
				if (but->isChecked ())
					result << but->property ("ListURL").value<QUrl> ();
			return result;
		}
	};

	void StartupFirstPage::handleAccepted ()
	{
		QList<QUrl> urlsToAdd;

		for (const auto box : findChildren<QGroupBox*> ())
			if (box->isChecked ())
			{
				urlsToAdd << box->property ("ListURL").value<QUrl> ();
				urlsToAdd << GetChildUrls (box);
			}

		for (const auto& url : urlsToAdd)
			Core_->Add (url);
	}
}
}
}
