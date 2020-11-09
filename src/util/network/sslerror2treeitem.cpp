/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sslerror2treeitem.h"
#include <QStringList>
#include <QDateTime>
#include <QTreeWidgetItem>
#include <QSslError>

namespace LC
{
namespace Util
{
	QTreeWidgetItem* SslError2TreeItem (const QSslError& error)
	{
		const auto item = new QTreeWidgetItem { { "Error:", error.errorString () } };

		const auto& cer = error.certificate ();
		if (cer.isNull ())
		{
			new QTreeWidgetItem
			{
				item,
				{ QObject::tr ("Certificate"), QObject::tr ("(No certificate available for this error)") }
			};
			return item;
		}

		new QTreeWidgetItem
		{
			item,
			{
				QObject::tr ("Valid:"),
				!cer.isBlacklisted () ? QObject::tr ("yes") : QObject::tr ("no")
			}
		};
		new QTreeWidgetItem { item, { QObject::tr ("Effective date:"), cer.effectiveDate ().toString () } };
		new QTreeWidgetItem { item, { QObject::tr ("Expiry date:"), cer.expiryDate ().toString () } };
		new QTreeWidgetItem { item, { QObject::tr ("Version:"), cer.version () } };
		new QTreeWidgetItem { item, { QObject::tr ("Serial number:"), cer.serialNumber () } };
		new QTreeWidgetItem { item, { QObject::tr ("MD5 digest:"), cer.digest ().toHex () } };
		new QTreeWidgetItem { item, { QObject::tr ("SHA1 digest:"), cer.digest (QCryptographicHash::Sha1).toHex () } };

		QString tmpString;
		auto cvt = [] (const QStringList& list) { return list.join ("; "_ql); };

		const auto issuer = new QTreeWidgetItem { item, { QObject::tr ("Issuer info") } };
		auto mkIssuerItem = [&cvt, &cer, issuer] (const QString& name,
				QSslCertificate::SubjectInfo field)
		{
			const auto& value = cvt (cer.issuerInfo (field));
			if (!value.isEmpty ())
				new QTreeWidgetItem { issuer, { name, value } };
		};

		mkIssuerItem (QObject::tr ("Organization:"), QSslCertificate::Organization);
		mkIssuerItem (QObject::tr ("Common name:"), QSslCertificate::CommonName);
		mkIssuerItem (QObject::tr ("Locality:"), QSslCertificate::LocalityName);
		mkIssuerItem (QObject::tr ("Organizational unit name:"), QSslCertificate::OrganizationalUnitName);
		mkIssuerItem (QObject::tr ("Country name:"), QSslCertificate::CountryName);
		mkIssuerItem (QObject::tr ("State or province name:"), QSslCertificate::StateOrProvinceName);

		const auto subject = new QTreeWidgetItem { item, { QObject::tr ("Subject info") } };
		auto mkSubjectItem = [&cvt, &cer, subject] (const QString& name,
				QSslCertificate::SubjectInfo field)
		{
			const auto& value = cvt (cer.subjectInfo (field));
			if (!value.isEmpty ())
				new QTreeWidgetItem { subject, { name, value } };
		};

		mkSubjectItem (QObject::tr ("Organization:"), QSslCertificate::Organization);
		mkSubjectItem (QObject::tr ("Common name:"), QSslCertificate::CommonName);
		mkSubjectItem (QObject::tr ("Locality:"), QSslCertificate::LocalityName);
		mkSubjectItem (QObject::tr ("Organizational unit name:"), QSslCertificate::OrganizationalUnitName);
		mkSubjectItem (QObject::tr ("Country name:"), QSslCertificate::CountryName);
		mkSubjectItem (QObject::tr ("State or province name:"), QSslCertificate::StateOrProvinceName);

		return item;
	}
}
}
