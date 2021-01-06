/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "manager.h"
#include <QFile>
#include <QDataStream>
#include <QSslConfiguration>
#include <QDir>
#include <QtDebug>
#include <util/util.h>
#include <util/sys/paths.h>
#include "certsmodel.h"

namespace LC
{
namespace CertMgr
{
	Manager::Manager ()
	: SystemCertsModel_ { new CertsModel { this } }
	, LocalCertsModel_ { new CertsModel { this } }
	{
		QSet<QByteArray> existing;
		for (const auto& cert : QSslConfiguration::systemCaCertificates ())
		{
			const auto& pem = cert.toPem ();
			if (existing.contains (pem))
				continue;

			existing << pem;
			Defaults_ << cert;
		}

		LoadLocals ();
		LoadBlacklist ();

		for (const auto& cert : Blacklist_)
			SystemCertsModel_->SetBlacklisted (cert, true);

		RegenAllowed ();

		ResetSocketDefault ();

		SystemCertsModel_->ResetCerts (Defaults_);
		LocalCertsModel_->ResetCerts (Locals_);
	}

	int Manager::AddCerts (const QList<QSslCertificate>& certs)
	{
		int added = 0;
		for (const auto& cert : certs)
		{
			if (cert.isNull ())
			{
				qWarning () << Q_FUNC_INFO
						<< "the certificate is null"
						<< cert;
				continue;
			}

			if (Defaults_.contains (cert) || Locals_.contains (cert))
			{
				qWarning () << Q_FUNC_INFO
						<< "certificate is already added";
				continue;
			}

			Locals_ << cert;
			LocalCertsModel_->AddCert (cert);
			++added;
		}

		if (!added)
			return 0;

		SaveLocals ();
		ResetSocketDefault ();

		return added;
	}

	void Manager::RemoveCert (const QSslCertificate& cert)
	{
		if (!Locals_.removeOne (cert))
			return;

		LocalCertsModel_->RemoveCert (cert);

		SaveLocals ();
		ResetSocketDefault ();
	}

	QAbstractItemModel* Manager::GetSystemModel () const
	{
		return SystemCertsModel_;
	}

	QAbstractItemModel* Manager::GetLocalModel () const
	{
		return LocalCertsModel_;
	}

	const QList<QSslCertificate>& Manager::GetLocalCerts () const
	{
		return Locals_;
	}

	const QList<QSslCertificate>& Manager::GetDefaultCerts () const
	{
		return Defaults_;
	}

	bool Manager::IsBlacklisted (const QSslCertificate& cert) const
	{
		return Blacklist_.contains (cert);
	}

	void Manager::ToggleBlacklist (const QSslCertificate& cert, bool blacklist)
	{
		if (cert.isNull ())
			return;

		if ((blacklist && Blacklist_.contains (cert)) ||
			(!blacklist && !Blacklist_.removeAll (cert)))
			return;

		if (blacklist)
			Blacklist_ << cert;

		SystemCertsModel_->SetBlacklisted (cert, blacklist);

		RegenAllowed ();
		ResetSocketDefault ();
	}

	void Manager::RegenAllowed ()
	{
		AllowedDefaults_.clear ();

		for (const auto& item : Defaults_)
			if (!Blacklist_.contains (item))
				AllowedDefaults_ << item;
	}

	void Manager::ResetSocketDefault ()
	{
		auto def = QSslConfiguration::defaultConfiguration ();
		def.setCaCertificates (AllowedDefaults_ + Locals_);
		QSslConfiguration::setDefaultConfiguration (def);
	}

	namespace
	{
		void Save (const QString& filename, const QList<QSslCertificate>& certs)
		{
			const auto& path = Util::CreateIfNotExists ("certmgr").absoluteFilePath (filename);

			QFile file { path };
			if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot open file"
						<< path
						<< file.errorString ();
				return;
			}

			QDataStream stream { &file };
			for (const auto& cert : certs)
				stream << cert.toPem ();
		}

		void Load (const QString& filename, QList<QSslCertificate>& certs)
		{
			const auto& path = Util::CreateIfNotExists ("certmgr").absoluteFilePath (filename);

			QFile file { path };
			if (!file.exists ())
				return;

			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot open file"
						<< path
						<< file.errorString ();
				return;
			}

			QDataStream stream { &file };

			while (!stream.atEnd ())
			{
				QByteArray ba;
				stream >> ba;

				certs << QSslCertificate::fromData (ba, QSsl::Pem);
			}
		}
	}

	void Manager::SaveBlacklist () const
	{
		Save ("blacklist", Blacklist_);
	}

	void Manager::LoadBlacklist ()
	{
		Load ("blacklist", Blacklist_);
	}

	void Manager::SaveLocals () const
	{
		Save ("locals", Locals_);
	}

	void Manager::LoadLocals ()
	{
		Load ("locals", Locals_);
	}
}
}
