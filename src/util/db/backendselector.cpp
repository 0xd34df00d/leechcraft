/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "backendselector.h"
#include <QSqlDatabase>
#include "ui_backendselector.h"
#include "../xmlsettingsdialog/basesettingsmanager.h"

namespace LC::Util
{
	BackendSelector::BackendSelector (BaseSettingsManager *m, QWidget *parent)
	: QWidget (parent)
	, Manager_ (m)
	{
		Ui_ = new Ui::BackendSelector;
		Ui_->setupUi (this);

		FillUI ();

		// We should check from last to first
		if (!QSqlDatabase::isDriverAvailable ("QMYSQL"))
		{
			Ui_->MySQLSettings_->setEnabled (false);
			Ui_->StorageType_->removeItem (2);
		}
		if (!QSqlDatabase::isDriverAvailable ("QPSQL"))
		{
			Ui_->PostgreSQLSettings_->setEnabled (false);
			Ui_->StorageType_->removeItem (1);
		}
	}

	void BackendSelector::FillUI ()
	{
		int index = Ui_->StorageType_->findText (Manager_->Property ("StorageType", "SQLite").toString ());
		Ui_->StorageType_->setCurrentIndex (index);
		Ui_->Settings_->setCurrentIndex (index);

		Ui_->PostgresHostname_->setText (Manager_->Property ("PostgresHostname", "localhost").toString ());
		Ui_->PostgresPort_->setValue (Manager_->Property ("PostgresPort", 5432).toInt ());
		Ui_->PostgresDBName_->setText (Manager_->Property ("PostgresDBName", "").toString ());
		Ui_->PostgresUsername_->setText (Manager_->Property ("PostgresUsername", "").toString ());
		Ui_->PostgresPassword_->setText (Manager_->Property ("PostgresPassword", "").toString ());

		Ui_->MysqlHostname_->setText (Manager_->Property ("MysqlHostname", "localhost").toString ());
		Ui_->MysqlPort_->setValue (Manager_->Property ("MysqlPort", 5432).toInt ());
		Ui_->MysqlDBName_->setText (Manager_->Property ("MysqlDBName", "").toString ());
		Ui_->MysqlUsername_->setText (Manager_->Property ("MysqlUsername", "").toString ());
		Ui_->MysqlPassword_->setText (Manager_->Property ("MysqlPassword", "").toString ());
	}

	void BackendSelector::accept ()
	{
		Manager_->setProperty ("StorageType", Ui_->StorageType_->currentText ());

		Manager_->setProperty ("PostgresHostname", Ui_->PostgresHostname_->text ());
		Manager_->setProperty ("PostgresPort", Ui_->PostgresPort_->value ());
		Manager_->setProperty ("PostgresDBName", Ui_->PostgresDBName_->text ());
		Manager_->setProperty ("PostgresUsername", Ui_->PostgresUsername_->text ());
		Manager_->setProperty ("PostgresPassword", Ui_->PostgresPassword_->text ());

		Manager_->setProperty ("MysqlHostname", Ui_->MysqlHostname_->text ());
		Manager_->setProperty ("MysqlPort", Ui_->MysqlPort_->value ());
		Manager_->setProperty ("MysqlDBName", Ui_->MysqlDBName_->text ());
		Manager_->setProperty ("MysqlUsername", Ui_->MysqlUsername_->text ());
		Manager_->setProperty ("MysqlPassword", Ui_->MysqlPassword_->text ());
	}

	void BackendSelector::reject ()
	{
		FillUI ();
	}
}

