/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "updatetypedialog.h"
#include <QDateTime>
#include <QStackedWidget>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	UpdateTypeDialog::UpdateTypeDialog (LoadType lt, QWidget *parent)
	: QDialog (parent)
	, LT_ (lt)
	{
		Ui_.setupUi (this);

		switch (LT_)
		{
		case LoadType::LoadLastEvents:
			Ui_.UpdatesWidgetStack_->setCurrentIndex (LoadLastEvents);
			Ui_.EntriesCount_->setValue (XmlSettingsManager::Instance ()
					.Property ("LoadEntriesToView", 20).toInt ());
			break;
		case LoadType::LoadChangesEvents:
			Ui_.UpdatesWidgetStack_->setCurrentIndex (LoadChangesEvents);
			Ui_.DateEdit_->setDateTime (XmlSettingsManager::Instance ()
					.Property ("ChangedDateToView",
						QDateTime::fromString ("01.01.1980 00:00", "dd.MM.yyyy hh:mm"))
							.toDateTime ());
			break;
		};
	}

	int UpdateTypeDialog::GetCount () const
	{
		return  Ui_.EntriesCount_->value ();
	}

	QDateTime UpdateTypeDialog::GetDateTime () const
	{
		return  Ui_.DateEdit_->dateTime ();
	}

	void UpdateTypeDialog::accept ()
	{
		switch (LT_)
		{
		case LoadType::LoadLastEvents:
			XmlSettingsManager::Instance ()
					.setProperty ("LoadEntriesToView", Ui_.EntriesCount_->value ());
			XmlSettingsManager::Instance ()
					.setProperty ("LoadLastAsk", !Ui_.UpdateAsk_->isChecked ());
			break;
		case LoadType::LoadChangesEvents:
			XmlSettingsManager::Instance ()
					.setProperty ("ChangedDateToView", QDateTime::currentDateTime ());
			XmlSettingsManager::Instance ()
					.setProperty ("LoadChangedAsk", !Ui_.UpdateAsk_->isChecked ());
			break;
		};
		QDialog::accept ();
	}

}
}
}
