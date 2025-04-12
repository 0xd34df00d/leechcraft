/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "startupsecondpage.h"
#include <util/db/backendselector.h>
#include "common.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Aggregator
{
	StartupSecondPage::StartupSecondPage (QWidget *parent)
	: QWizardPage (parent)
	, Selector_ (new Util::BackendSelector (&XmlSettingsManager::Instance ()))
	{
		Ui_.setupUi (this);
		QHBoxLayout *lay = new QHBoxLayout ();
		lay->addWidget (Selector_);
		Ui_.SelectorContainer_->setLayout (lay);

		setTitle (PluginVisibleName);
		setSubTitle (tr ("Set storage options"));

		setProperty ("WizardType", 1);
	}

	void StartupSecondPage::initializePage ()
	{
		connect (wizard (),
				&QWizard::accepted,
				Selector_,
				&Util::BackendSelector::accept,
				Qt::UniqueConnection);
		XmlSettingsManager::Instance ().setProperty ("StartupVersion", 2);

		wizard ()->setField ("Aggregator/StorageDirty", true);
	}
}
}
