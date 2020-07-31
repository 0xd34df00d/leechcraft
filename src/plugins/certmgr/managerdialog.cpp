/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "managerdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <util/sll/prelude.h>
#include "manager.h"
#include "certsmodel.h"

namespace LC
{
namespace CertMgr
{
	ManagerDialog::ManagerDialog (Manager *manager, QWidget *parent)
	: QDialog { parent }
	, Manager_ { manager }
	{
		Ui_.setupUi (this);

		Ui_.SystemTree_->setModel (manager->GetSystemModel ());
		Ui_.LocalTree_->setModel (manager->GetLocalModel ());

		connect (Ui_.SystemTree_->selectionModel (),
				SIGNAL (selectionChanged (QItemSelection, QItemSelection)),
				this,
				SLOT (updateSystemButtons ()));
		updateSystemButtons ();

		connect (Ui_.LocalTree_->selectionModel (),
				SIGNAL (selectionChanged (QItemSelection, QItemSelection)),
				this,
				SLOT (updateLocalButtons ()));
		updateLocalButtons ();
	}

	QSslCertificate ManagerDialog::GetSelectedCert (CertPart part) const
	{
		QTreeView *view = nullptr;
		switch (part)
		{
		case CertPart::System:
			view = Ui_.SystemTree_;
			break;
		case CertPart::Local:
			view = Ui_.LocalTree_;
			break;
		}

		if (!view)
			return QSslCertificate {};

		const auto& selected = view->selectionModel ()->selectedRows ();
		return selected.value (0).data (CertsModel::CertificateRole).value<QSslCertificate> ();
	}

	void ManagerDialog::on_AddLocal__released ()
	{
		const auto& paths = QFileDialog::getOpenFileNames (this,
				tr ("Select certificate files"),
				QDir::homePath (),
				"Certificates (*.crt *.pem);;All files (*)");
		if (paths.isEmpty ())
			return;

		using CertPair_t = QPair<QSslCertificate, QByteArray>;
		QList<CertPair_t> certs;
		for (const auto& path : paths)
		{
			const auto addCert = [&certs, &path] (QSsl::EncodingFormat type)
			{
				for (const auto& cert : QSslCertificate::fromPath (path, type))
					if (!cert.isNull ())
						certs.append ({ cert, cert.digest () });
			};
			addCert (QSsl::Pem);
			addCert (QSsl::Der);
		}

		certs.erase (std::remove_if (certs.begin (), certs.end (),
					[] (const CertPair_t& p) { return p.first.isNull (); }),
				certs.end ());

		const auto comparator = [] (const CertPair_t& p1, const CertPair_t& p2)
				{ return p1.second < p2.second; };
		std::sort (certs.begin (), certs.end (), comparator);
		certs.erase (std::unique (certs.begin (), certs.end (), comparator), certs.end ());

		if (certs.isEmpty ())
		{
			QMessageBox::warning (this,
					tr ("Certificates import"),
					tr ("No valid certificates could be found."));
			return;
		}

		const auto numAdded = Manager_->AddCerts (Util::Map (certs,
					[] (const CertPair_t& p) { return p.first; }));

		if (!numAdded)
			QMessageBox::warning (this,
					tr ("Certificates import"),
					tr ("No certificates were added. Very likely all of them are "
						"already present in the system certificates database."));
		else if (paths.size () > 1 ||
				QFileInfo { paths.value (0) }.isDir ())
			QMessageBox::information (this,
					tr ("Certificates import"),
					tr ("%n certificate(s) were added.", 0, numAdded));
	}

	void ManagerDialog::on_RemoveLocal__released ()
	{
		const auto& selectedCert = GetSelectedCert (CertPart::Local);
		if (!selectedCert.isNull ())
			Manager_->RemoveCert (selectedCert);
		else
		{
			const auto& selected = Ui_.LocalTree_->selectionModel ()->selectedRows ().value (0);
			const auto model = Manager_->GetLocalModel ();
			for (auto i = 0, rc = model->rowCount (selected); i < rc; ++i)
			{
				const auto& child = model->index (i, 0, selected);
				const auto& certVar = child.data (CertsModel::CertificateRole);
				Manager_->RemoveCert (certVar.value<QSslCertificate> ());
			}
		}
	}

	void ManagerDialog::updateLocalButtons ()
	{
		const auto& cert = GetSelectedCert (CertPart::Local);
		const auto& selected = Ui_.LocalTree_->selectionModel ()->selectedRows ();
		Ui_.RemoveLocal_->setEnabled (!cert.isNull () || !selected.isEmpty ());
	}

	void ManagerDialog::on_Enable__released ()
	{
		Manager_->ToggleBlacklist (GetSelectedCert (CertPart::System), false);
		updateSystemButtons ();
	}

	void ManagerDialog::on_Disable__released ()
	{
		Manager_->ToggleBlacklist (GetSelectedCert (CertPart::System), true);
		updateSystemButtons ();
	}

	void ManagerDialog::updateSystemButtons ()
	{
		const auto& cert = GetSelectedCert (CertPart::System);

		if (cert.isNull ())
		{
			Ui_.Enable_->setEnabled (false);
			Ui_.Disable_->setEnabled (false);
			return;
		}

		const auto isBl = Manager_->IsBlacklisted (cert);
		Ui_.Enable_->setEnabled (isBl);
		Ui_.Disable_->setEnabled (!isBl);
	}
}
}
