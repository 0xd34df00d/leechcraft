/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "backendselector.h"
#include <QSqlDatabase>
#include "ui_backendselector.h"

using namespace LeechCraft::Util;

BackendSelector::BackendSelector (BaseSettingsManager *m,
		QWidget *parent)
: QWidget (parent)
, Manager_ (m)
{
	Ui_ = new Ui::BackendSelector;
	Ui_->setupUi (this);

	FillUI ();

	// We should check from last to first
	if (!QSqlDatabase::isDriverAvailable ("QPSQL"))
	{
		Ui_->PostgreSQLSettings_->setEnabled (false);
		Ui_->StorageType_->removeItem (1);
	}
}

void BackendSelector::FillUI ()
{
	int index = Ui_->StorageType_->
			findText (Manager_->
				Property ("StorageType", "SQLite").toString ());
	Ui_->StorageType_->setCurrentIndex (index);
	Ui_->Settings_->setCurrentIndex (index);

	Ui_->SQLiteVacuum_->setCheckState (Manager_->
			Property ("SQLiteVacuum", false).toBool () ?
			Qt::Checked :
			Qt::Unchecked);
	Ui_->SQLiteJournalMode_->setCurrentIndex (Ui_->SQLiteJournalMode_->
			findText (Manager_->
				Property ("SQLiteJournalMode", "TRUNCATE").toString ()));
	Ui_->SQLiteTempStore_->setCurrentIndex (Ui_->SQLiteTempStore_->
			findText (Manager_->
				Property ("SQLiteTempStore", "MEMORY").toString ()));
	Ui_->SQLiteSynchronous_->setCurrentIndex (Ui_->SQLiteSynchronous_->
			findText (Manager_->
				Property ("SQLiteSynchronous", "OFF").toString ()));

	Ui_->PostgresHostname_->setText (Manager_->
			Property ("PostgresHostname", "localhost").toString ());
	Ui_->PostgresPort_->setValue (Manager_->
			Property ("PostgresPort", 5432).toInt ());
	Ui_->PostgresDBName_->setText (Manager_->
			Property ("PostgresDBName", "").toString ());
	Ui_->PostgresUsername_->setText (Manager_->
			Property ("PostgresUsername", "").toString ());
	Ui_->PostgresPassword_->setText (Manager_->
			Property ("PostgresPassword", "").toString ());
}

void BackendSelector::accept ()
{
	Manager_->setProperty ("StorageType",
			Ui_->StorageType_->currentText ());

	Manager_->setProperty ("SQLiteVacuum",
			Ui_->SQLiteVacuum_->checkState () == Qt::Checked);
	Manager_->setProperty ("SQLiteJournalMode",
			Ui_->SQLiteJournalMode_->currentText ());
	Manager_->setProperty ("SQLiteTempStore",
			Ui_->SQLiteTempStore_->currentText ());
	Manager_->setProperty ("SQLiteSynchronous",
			Ui_->SQLiteSynchronous_->currentText ());

	Manager_->setProperty ("PostgresHostname",
			Ui_->PostgresHostname_->text ());
	Manager_->setProperty ("PostgresPort",
			Ui_->PostgresPort_->value ());
	Manager_->setProperty ("PostgresDBName",
			Ui_->PostgresDBName_->text ());
	Manager_->setProperty ("PostgresUsername",
			Ui_->PostgresUsername_->text ());
	Manager_->setProperty ("PostgresPassword",
			Ui_->PostgresPassword_->text ());
}

void BackendSelector::reject ()
{
	FillUI ();
}

