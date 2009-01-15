#include "sslerrorsdialog.h"
#include <QDateTime>

using namespace LeechCraft;

SslErrorsDialog::SslErrorsDialog (const QString& msg,
		const QList<QSslError>& errors,
		QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
	Ui_.Description_->setText (msg);
	for (QList<QSslError>::const_iterator i = errors.begin (),
			end = errors.end (); i != end; ++i)
		PopulateTree (*i);
	Ui_.Errors_->expandAll ();
}

SslErrorsDialog::~SslErrorsDialog ()
{
}

SslErrorsDialog::RememberChoice SslErrorsDialog::GetRememberChoice () const
{
	if (Ui_.RememberNot_->isChecked ())
		return RCNot;
	else if (Ui_.RememberFile_->isChecked ())
		return RCFile;
	else
		return RCHost;
}

void SslErrorsDialog::PopulateTree (const QSslError& error)
{
	QTreeWidgetItem *item = new QTreeWidgetItem (Ui_.Errors_,
			QStringList ("Error:") << error.errorString ());

	QSslCertificate cer = error.certificate ();
	if (cer.isNull ())
	{
		new QTreeWidgetItem (item,
				QStringList (tr ("Certificate")) <<
					tr ("(No certificate available for this error)"));
		return;
	}

	new QTreeWidgetItem (item, QStringList (tr ("Valid:")) <<
				(cer.isValid () ? tr ("yes") : tr ("no")));
	new QTreeWidgetItem (item, QStringList (tr ("Effective date:")) <<
				cer.effectiveDate ().toString ());
	new QTreeWidgetItem (item, QStringList (tr ("Expiry date:")) <<
				cer.expiryDate ().toString ());
	new QTreeWidgetItem (item, QStringList (tr ("Version:")) <<
				cer.version ());
	new QTreeWidgetItem (item, QStringList (tr ("Serial number:")) <<
				cer.serialNumber ());
	new QTreeWidgetItem (item, QStringList (tr ("MD5 digest:")) <<
				cer.digest ().toHex ());
	new QTreeWidgetItem (item, QStringList (tr ("SHA1 digest:")) <<
				cer.digest (QCryptographicHash::Sha1).toHex ());

	QTreeWidgetItem *issuer = new QTreeWidgetItem (item,
			QStringList (tr ("Issuer info")));

	QString tmpString;
	tmpString = cer.issuerInfo (QSslCertificate::Organization);
	if (!tmpString.isEmpty ())
		new QTreeWidgetItem (issuer,
				QStringList (tr ("Organization:")) << tmpString);

	tmpString = cer.issuerInfo (QSslCertificate::CommonName);
	if (!tmpString.isEmpty ())
		new QTreeWidgetItem (issuer,
				QStringList (tr ("Common name:")) << tmpString);

	tmpString = cer.issuerInfo (QSslCertificate::LocalityName);
	if (!tmpString.isEmpty ())
		new QTreeWidgetItem (issuer,
				QStringList (tr ("Locality:")) << tmpString);

	tmpString = cer.issuerInfo (QSslCertificate::OrganizationalUnitName);
	if (!tmpString.isEmpty ())
		new QTreeWidgetItem (issuer,
				QStringList (tr ("Organizational unit name:")) << tmpString);

	tmpString = cer.issuerInfo (QSslCertificate::CountryName);
	if (!tmpString.isEmpty ())
		new QTreeWidgetItem (issuer,
				QStringList (tr ("Country name:")) << tmpString);

	tmpString = cer.issuerInfo (QSslCertificate::StateOrProvinceName);
	if (!tmpString.isEmpty ())
		new QTreeWidgetItem (issuer,
				QStringList (tr ("State or province name:")) << tmpString);

	QTreeWidgetItem *subject = new QTreeWidgetItem (item,
			QStringList (tr ("Subject info")));

	tmpString = cer.subjectInfo (QSslCertificate::Organization);
	if (!tmpString.isEmpty ())
		new QTreeWidgetItem (subject,
				QStringList (tr ("Organization:")) << tmpString);

	tmpString = cer.subjectInfo (QSslCertificate::CommonName);
	if (!tmpString.isEmpty ())
		new QTreeWidgetItem (subject,
				QStringList (tr ("Common name:")) << tmpString);

	tmpString = cer.subjectInfo (QSslCertificate::LocalityName);
	if (!tmpString.isEmpty ())
		new QTreeWidgetItem (subject,
				QStringList (tr ("Locality:")) << tmpString);

	tmpString = cer.subjectInfo (QSslCertificate::OrganizationalUnitName);
	if (!tmpString.isEmpty ())
		new QTreeWidgetItem (subject,
				QStringList (tr ("Organizational unit name:")) << tmpString);

	tmpString = cer.subjectInfo (QSslCertificate::CountryName);
	if (!tmpString.isEmpty ())
		new QTreeWidgetItem (subject,
				QStringList (tr ("Country name:")) << tmpString);

	tmpString = cer.subjectInfo (QSslCertificate::StateOrProvinceName);
	if (!tmpString.isEmpty ())
		new QTreeWidgetItem (subject,
				QStringList (tr ("State or province name:")) << tmpString);
}

