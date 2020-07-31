/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QAbstractItemModel>
#include <QSslCertificate>

namespace LC
{
namespace CertMgr
{
	class CertsModel : public QAbstractItemModel
	{
		typedef QList<QPair<QString, QList<QSslCertificate>>> CertsDict_t;
		CertsDict_t Issuer2Certs_;

		QList<QSslCertificate> Blacklisted_;
	public:
		enum Role
		{
			CertificateRole = Qt::UserRole + 1
		};

		using QAbstractItemModel::QAbstractItemModel;

		QModelIndex index (int row, int column, const QModelIndex& parent) const;
		QModelIndex parent (const QModelIndex& child) const;
		int columnCount (const QModelIndex& parent) const;
		int rowCount (const QModelIndex& parent) const;
		QVariant data (const QModelIndex& index, int role) const;

		void AddCert (const QSslCertificate&);
		void RemoveCert (const QSslCertificate&);

		void ResetCerts (const QList<QSslCertificate>&);

		QModelIndex FindCertificate (const QSslCertificate&) const;

		void SetBlacklisted (const QSslCertificate&, bool blacklisted);
	private:
		CertsDict_t::iterator GetListPosForCert (const QSslCertificate&);
		CertsDict_t::const_iterator GetListPosForCert (const QSslCertificate&) const;

		CertsDict_t::iterator CreateListPosForCert (const QSslCertificate&);
		QList<QSslCertificate>& CreateListForCert (const QSslCertificate&);
	};
}
}
