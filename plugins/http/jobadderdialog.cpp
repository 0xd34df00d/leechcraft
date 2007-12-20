#include <QtGui>
#include <QtDebug>
#include "jobadderdialog.h"
#include "jobparams.h"
#include "settingsmanager.h"

JobAdderDialog::JobAdderDialog (QWidget *parent)
: QDialog (parent)
{
	URLLabel_ = new QLabel (tr ("URL:"));
	LocalLabel_ = new QLabel (tr ("Local path:"));

	QString defaultURL = QApplication::clipboard ()->text ();
	if (defaultURL.isEmpty () || (defaultURL.left (3) != "ftp" && defaultURL.left (4) != "http"))
		defaultURL = "http://gentoo.osuosl.org/distfiles/vsftpd-2.0.5.tar.gz";
	URL_ = new QLineEdit (defaultURL);
	
	QString dd = SettingsManager::Instance ()->GetDownloadDir ();
	if (dd == "")
	{
		dd = QDir::homePath ();
	}
	LocalName_ = new QLineEdit (dd);

	QPushButton *browseButton = new QPushButton (tr ("Browse..."));
	connect (browseButton, SIGNAL (released ()), this, SLOT (selectDir ()));

	OK_ = new QPushButton (tr ("OK"));
	Cancel_ = new QPushButton (tr ("Cancel"));

	connect (OK_, SIGNAL (released ()), this, SLOT (accept ()));
	connect (Cancel_, SIGNAL (released ()), this, SLOT (reject ()));

	Autostart_ = new QCheckBox (tr ("Autostart"));

	OK_->setDefault (true);

	QHBoxLayout *URLLay_ = new QHBoxLayout;
	URLLay_->addWidget (URLLabel_);
	URLLay_->addWidget (URL_);

	QHBoxLayout *localLay_ = new QHBoxLayout;
	localLay_->addWidget (LocalLabel_);
	localLay_->addWidget (LocalName_);
	localLay_->addWidget (browseButton);

	QHBoxLayout *ctrlButtonsLay_ = new QHBoxLayout;
	ctrlButtonsLay_->addWidget (OK_);
	ctrlButtonsLay_->addWidget (Cancel_);

	QVBoxLayout *mainLayout_ = new QVBoxLayout;
	mainLayout_->addLayout (URLLay_);
	mainLayout_->addLayout (localLay_);
	mainLayout_->addWidget (Autostart_);
	mainLayout_->addLayout (ctrlButtonsLay_);

	setLayout (mainLayout_);
}

void JobAdderDialog::done (int r)
{
	if (r == Accepted)
	{
		JobParams *p = new JobParams;
		p->URL_ = URL_->text ();
		p->LocalName_ = LocalName_->text ();
		p->IsFullName_ = false;
		p->Autostart_ = (Autostart_->checkState () == Qt::Checked);
		p->ShouldBeSavedInHistory_ = true;

		emit gotParams (p);
	}
	QDialog::done (r);
}

void JobAdderDialog::selectDir ()
{
	LocalName_->setText (QFileDialog::getExistingDirectory (parentWidget (), tr ("Select directory")));
}

