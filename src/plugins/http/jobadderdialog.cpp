#include <QtGui/QtGui>
#include <QtDebug>
#include "jobadderdialog.h"
#include "jobparams.h"
#include "xmlsettingsmanager.h"

JobAdderDialog::JobAdderDialog (QWidget *parent)
: QDialog (parent)
{
	setupUi (this);

	QString defaultURL = QApplication::clipboard ()->text ();
	if (defaultURL.isEmpty () || (defaultURL.left (3) != "ftp" && defaultURL.left (4) != "http"))
		defaultURL = "http://gentoo.osuosl.org/distfiles/vsftpd-2.0.5.tar.gz";
	URL_->setText (defaultURL);
	
	QString dd = XmlSettingsManager::Instance ()->property ("DownloadDir").toString ();
	if (dd == "")
		dd = QDir::homePath ();
	LocalName_->setText (dd);
}

void JobAdderDialog::SetURL (const QString& url)
{
	URL_->setText (url);
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
		p->Size_ = 0;
		p->DownloadTime_ = 0;

		emit gotParams (p);
	}
	QDialog::done (r);
}

void JobAdderDialog::on_BrowseButton__released ()
{
	QString dir = QFileDialog::getExistingDirectory (parentWidget (), tr ("Select directory"), LocalName_->text ());
	if (!dir.isEmpty ())
		LocalName_->setText (dir);
}

