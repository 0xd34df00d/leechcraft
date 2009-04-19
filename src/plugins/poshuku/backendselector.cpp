#include "backendselector.h"
#include "xmlsettingsmanager.h"

BackendSelector::BackendSelector (QWidget *parent)
: QWidget (parent)
{
	Ui_.setupUi (this);

	FillUI ();
}

void BackendSelector::FillUI ()
{
	int index = Ui_.StorageType_->
			findText (XmlSettingsManager::Instance ()->
				Property ("StorageType", "SQLite").toString ());
	Ui_.StorageType_->setCurrentIndex (index);
	Ui_.Settings_->setCurrentIndex (index);

	Ui_.SQLiteVacuum_->setCheckState (XmlSettingsManager::Instance ()->
			Property ("SQLiteVacuum", false).toBool () ?
			Qt::Checked :
			Qt::Unchecked);
	Ui_.SQLiteJournalMode_->setCurrentIndex (Ui_.SQLiteJournalMode_->
			findText (XmlSettingsManager::Instance ()->
				Property ("SQLiteJournalMode", "TRUNCATE").toString ()));
	Ui_.SQLiteTempStore_->setCurrentIndex (Ui_.SQLiteTempStore_->
			findText (XmlSettingsManager::Instance ()->
				Property ("SQLiteTempStore", "MEMORY").toString ()));
	Ui_.SQLiteSynchronous_->setCurrentIndex (Ui_.SQLiteSynchronous_->
			findText (XmlSettingsManager::Instance ()->
				Property ("SQLiteSynchronous", "OFF").toString ()));

	Ui_.PostgresHostname_->setText (XmlSettingsManager::Instance ()->
			Property ("PostgresHostname", "localhost").toString ());
	Ui_.PostgresPort_->setValue (XmlSettingsManager::Instance ()->
			Property ("PostgresPort", 5432).toInt ());
	Ui_.PostgresDBName_->setText (XmlSettingsManager::Instance ()->
			Property ("PostgresDBName", "lc_poshuku").toString ());
	Ui_.PostgresUsername_->setText (XmlSettingsManager::Instance ()->
			Property ("PostgresUsername", "").toString ());
	Ui_.PostgresPassword_->setText (XmlSettingsManager::Instance ()->
			Property ("PostgresPassword", "").toString ());
}

void BackendSelector::accept ()
{
	XmlSettingsManager::Instance ()->setProperty ("StorageType",
			Ui_.StorageType_->currentText ());

	XmlSettingsManager::Instance ()->setProperty ("SQLiteVacuum",
			Ui_.SQLiteVacuum_->checkState () == Qt::Checked);
	XmlSettingsManager::Instance ()->setProperty ("SQLiteJournalMode",
			Ui_.SQLiteJournalMode_->currentText ());
	XmlSettingsManager::Instance ()->setProperty ("SQLiteTempStore",
			Ui_.SQLiteTempStore_->currentText ());
	XmlSettingsManager::Instance ()->setProperty ("SQLiteSynchronous",
			Ui_.SQLiteSynchronous_->currentText ());

	XmlSettingsManager::Instance ()->setProperty ("PostgresHostname",
			Ui_.PostgresHostname_->text ());
	XmlSettingsManager::Instance ()->setProperty ("PostgresPort",
			Ui_.PostgresPort_->value ());
	XmlSettingsManager::Instance ()->setProperty ("PostgresDBName",
			Ui_.PostgresDBName_->text ());
	XmlSettingsManager::Instance ()->setProperty ("PostgresUsername",
			Ui_.PostgresUsername_->text ());
	XmlSettingsManager::Instance ()->setProperty ("PostgresPassword",
			Ui_.PostgresPassword_->text ());
}

void BackendSelector::reject ()
{
	FillUI ();
}

