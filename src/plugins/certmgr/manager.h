/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSslCertificate>

class QAbstractItemModel;

namespace LC
{
namespace CertMgr
{
	class CertsModel;

	class Manager : public QObject
	{
		Q_OBJECT

		QList<QSslCertificate> Defaults_;
		QList<QSslCertificate> Blacklist_;
		QList<QSslCertificate> AllowedDefaults_;

		QList<QSslCertificate> Locals_;

		CertsModel * const SystemCertsModel_;
		CertsModel * const LocalCertsModel_;
	public:
		Manager ();

		int AddCerts (const QList<QSslCertificate>&);
		void RemoveCert (const QSslCertificate&);

		QAbstractItemModel* GetSystemModel () const;
		QAbstractItemModel* GetLocalModel () const;

		const QList<QSslCertificate>& GetLocalCerts () const;
		const QList<QSslCertificate>& GetDefaultCerts () const;

		bool IsBlacklisted (const QSslCertificate&) const;
		void ToggleBlacklist (const QSslCertificate&, bool blacklist);
	private:
		void RegenAllowed ();
		void ResetSocketDefault ();

		void SaveBlacklist () const;
		void LoadBlacklist ();

		void SaveLocals () const;
		void LoadLocals ();
	};
}
}
