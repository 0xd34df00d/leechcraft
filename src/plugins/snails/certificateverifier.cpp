/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "certificateverifier.h"
#include <QSslCertificate>
#include <QSslError>
#include <QtDebug>
#include <vmime/security/cert/certificateExpiredException.hpp>
#include <vmime/security/cert/certificateNotYetValidException.hpp>
#include <vmime/security/cert/serverIdentityException.hpp>
#include "vmimeconversions.h"

namespace LC
{
namespace Snails
{
	void CertificateVerifier::verify (const vmime::shared_ptr<vmime::security::cert::certificateChain>& chain,
			const vmime::string& host)
	{
		namespace vsc = vmime::security::cert;

		QList<QSslCertificate> qtChain;
		for (size_t i = 0; i < chain->getCount (); ++i)
		{
			const auto& item = chain->getAt (i);

			const auto& subcerts = ToSslCerts (item);
			if (subcerts.size () != 1)
			{
				qWarning () << Q_FUNC_INFO
						<< "unexpected certificates count for certificate type"
						<< item->getType ().c_str ()
						<< ", got"
						<< subcerts.size ()
						<< "certificates";
				throw vsc::unsupportedCertificateTypeException { "unexpected certificates counts" };
			}

			qtChain << subcerts.at (0);
		}

		const auto& errs = QSslCertificate::verify (qtChain, QString::fromStdString (host));
		if (errs.isEmpty ())
			return;

		qWarning () << Q_FUNC_INFO
				<< errs;

		for (const auto& error : errs)
			switch (error.error ())
			{
			case QSslError::CertificateExpired:
				throw vsc::certificateExpiredException {};
			case QSslError::CertificateNotYetValid:
				throw vsc::certificateNotYetValidException {};
			case QSslError::HostNameMismatch:
				throw vsc::serverIdentityException {};
			case QSslError::UnableToDecryptCertificateSignature:
			case QSslError::InvalidNotAfterField:
			case QSslError::InvalidNotBeforeField:
			case QSslError::CertificateSignatureFailed:
			case QSslError::PathLengthExceeded:
			case QSslError::UnspecifiedError:
				throw vsc::unsupportedCertificateTypeException { "incorrect format" };
			case QSslError::UnableToGetIssuerCertificate:
			case QSslError::UnableToGetLocalIssuerCertificate:
			case QSslError::UnableToDecodeIssuerPublicKey:
			case QSslError::UnableToVerifyFirstCertificate:
			case QSslError::SubjectIssuerMismatch:
			case QSslError::AuthorityIssuerSerialNumberMismatch:
				throw vsc::certificateIssuerVerificationException {};
			case QSslError::SelfSignedCertificate:
			case QSslError::SelfSignedCertificateInChain:
			case QSslError::CertificateRevoked:
			case QSslError::InvalidCaCertificate:
			case QSslError::InvalidPurpose:
			case QSslError::CertificateUntrusted:
			case QSslError::CertificateRejected:
			case QSslError::NoPeerCertificate:
			case QSslError::CertificateBlacklisted:
				throw vsc::certificateNotTrustedException {};
			case QSslError::NoError:
			case QSslError::NoSslSupport:
				break;
			}

		throw vsc::certificateException { "other certificate error" };
	}
}
}
