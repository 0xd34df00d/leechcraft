/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "updatetypedialog.h"
#include <QDateTime>
#include <QStackedWidget>
#include "xmlsettingsmanager.h"

namespace LeechCraft
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
