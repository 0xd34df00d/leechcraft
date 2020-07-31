/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "proxiesconfigwidget.h"
#include <QStandardItemModel>
#include <QSettings>
#include <QMessageBox>
#include <util/sll/slotclosure.h>
#include "proxiesstorage.h"
#include "editurlsdialog.h"
#include "editlistsdialog.h"
#include "scriptsmanager.h"
#include "proxyconfigdialog.h"
#include "proxiesstorage.h"

namespace LC
{
namespace XProxy
{
	namespace
	{
		QString ProxyType2Str (QNetworkProxy::ProxyType type)
		{
			switch (type)
			{
			case QNetworkProxy::ProxyType::Socks5Proxy:
				return "SOCKS5";
				break;
			case QNetworkProxy::ProxyType::HttpProxy:
				return "HTTP";
				break;
			case QNetworkProxy::ProxyType::HttpCachingProxy:
				return ProxiesConfigWidget::tr ("caching HTTP");
				break;
			case QNetworkProxy::ProxyType::FtpCachingProxy:
				return ProxiesConfigWidget::tr ("caching FTP");
				break;
			case QNetworkProxy::ProxyType::NoProxy:
				return ProxiesConfigWidget::tr ("no proxy");
				break;
			default:
				return ProxiesConfigWidget::tr ("other type");
				break;
			}
		}

		QList<QStandardItem*> Proxy2Row (const Proxy& proxy)
		{
			return
			{
				new QStandardItem { ProxyType2Str (proxy.Type_) },
				new QStandardItem { proxy.Host_ + ":" + QString::number (proxy.Port_) },
				new QStandardItem { proxy.User_ }
			};
		}
	}

	ProxiesConfigWidget::ProxiesConfigWidget (ProxiesStorage *storage,
			ScriptsManager *manager, QWidget *parent)
	: QWidget (parent)
	, Storage_ (storage)
	, ScriptsMgr_ (manager)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.ProxiesList_->setModel (Model_);

		connect (Ui_.ProxiesList_->selectionModel (),
				SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleItemSelected (QModelIndex)));

		const QStringList labels
		{
			tr ("Proxy type"),
			tr ("Proxy target"),
			tr ("User")
		};
		Model_->setHorizontalHeaderLabels (labels);

		reject ();
	}

	void ProxiesConfigWidget::accept ()
	{
		Storage_->SaveSettings ();
	}

	void ProxiesConfigWidget::reject ()
	{
		if (const auto rc = Model_->rowCount ())
			Model_->removeRows (0, rc);

		Storage_->LoadSettings ();

		Proxies_ = Storage_->GetKnownProxies ();
		for (const auto& proxy : Proxies_)
			Model_->appendRow (Proxy2Row (proxy));
	}

	void ProxiesConfigWidget::handleItemSelected (const QModelIndex& idx)
	{
		Ui_.UpdateProxyButton_->setEnabled (idx.isValid ());
		Ui_.RemoveProxyButton_->setEnabled (idx.isValid ());
		Ui_.EditUrlsButton_->setEnabled (idx.isValid ());
		Ui_.EditListsButton_->setEnabled (idx.isValid ());

		if (idx.isValid ())
		{
			Ui_.MoveUpButton_->setEnabled (idx.row () > 0);
			Ui_.MoveDownButton_->setEnabled (idx.row () + 1 < Model_->rowCount ());
		}
		else
		{
			Ui_.MoveUpButton_->setEnabled (false);
			Ui_.MoveDownButton_->setEnabled (false);
		}
	}

	void ProxiesConfigWidget::on_AddProxyButton__released ()
	{
		Proxy proxy;

		ProxyConfigDialog dia { this };
		int counter = 0;
		while (true)
		{
			if (dia.exec () != QDialog::Accepted)
				return;

			proxy = dia.GetProxy ();
			if (!Proxies_.contains (proxy))
				break;

			if (++counter == 5)
			{
				QMessageBox::critical (this,
						"HAL 9000",
						"I'm sorry Dave, I'm afraid I can't do that");
				return;
			}

			if (QMessageBox::question (this,
						"LeechCraft",
						tr ("The specified proxy already exists. "
							"Do you want to change the parameters of the new one?"),
						QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
				return;
		}

		Proxies_ << proxy;
		Model_->appendRow (Proxy2Row (proxy));

		Storage_->AddProxy (proxy);
	}

	void ProxiesConfigWidget::on_UpdateProxyButton__released ()
	{
		const int row = Ui_.ProxiesList_->currentIndex ().row ();
		if (row < 0 || row >= Proxies_.size ())
			return;

		const auto oldProxy = Proxies_.at (row);

		Proxy proxy;
		ProxyConfigDialog dia { this };
		dia.SetProxy (oldProxy);

		while (true)
		{
			if (dia.exec () != QDialog::Accepted)
				return;

			proxy = dia.GetProxy ();
			if (proxy == oldProxy)
				return;

			if (!Proxies_.contains (proxy))
				break;

			if (QMessageBox::question (this,
						"LeechCraft",
						tr ("The specified proxy already exists. "
							"Do you want to change the parameters of the new one?"),
						QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
				return;
		}

		Proxies_ [row] = proxy;

		const auto& newRow = Proxy2Row (proxy);
		for (int i = 0; i < newRow.size (); ++i)
			Model_->setItem (row, i, newRow.at (i));

		Storage_->UpdateProxy (oldProxy, proxy);
	}

	void ProxiesConfigWidget::on_RemoveProxyButton__released ()
	{
		const int row = Ui_.ProxiesList_->currentIndex ().row ();
		if (row < 0 || row >= Proxies_.size ())
			return;

		Model_->removeRow (row);
		Storage_->RemoveProxy (Proxies_.takeAt (row));
	}

	void ProxiesConfigWidget::on_MoveUpButton__released ()
	{
		const int row = Ui_.ProxiesList_->currentIndex ().row ();
		if (row <= 0)
			return;

		Model_->insertRow (row, Model_->takeRow (row - 1));
		Storage_->Swap (row, row - 1);

		handleItemSelected (Ui_.ProxiesList_->currentIndex ());
	}

	void ProxiesConfigWidget::on_MoveDownButton__released ()
	{
		const int row = Ui_.ProxiesList_->currentIndex ().row ();
		if (row + 1 >= Model_->rowCount ())
			return;

		Model_->insertRow (row, Model_->takeRow (row + 1));
		Storage_->Swap (row, row + 1);

		handleItemSelected (Ui_.ProxiesList_->currentIndex ());
	}

	void ProxiesConfigWidget::on_EditUrlsButton__released ()
	{
		const int row = Ui_.ProxiesList_->currentIndex ().row ();
		if (row < 0 || row >= Proxies_.size ())
			return;

		const auto& proxy = Proxies_.value (row);
		auto dialog = new EditUrlsDialog { Storage_->GetTargets (proxy), this };
		dialog->setAttribute (Qt::WA_DeleteOnClose);

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[this, proxy, dialog] { Storage_->SetTargets (proxy, dialog->GetTargets ()); },
			dialog,
			SIGNAL (accepted ()),
			dialog
		};

		dialog->show ();
	}

	void ProxiesConfigWidget::on_EditListsButton__released ()
	{
		const int row = Ui_.ProxiesList_->currentIndex ().row ();
		if (row < 0 || row >= Proxies_.size ())
			return;

		const auto& proxy = Proxies_.value (row);
		auto dialog = new EditListsDialog { Storage_->GetScripts (proxy), ScriptsMgr_, this };
		dialog->setAttribute (Qt::WA_DeleteOnClose);

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[this, proxy, dialog] { Storage_->SetScripts (proxy, dialog->GetScripts ()); },
			dialog,
			SIGNAL (accepted ()),
			dialog
		};

		dialog->show ();
	}
}
}
