/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "core.h"
#include <QStandardItemModel>
#include <QSettings>
#include <QCoreApplication>
#include <QtDebug>

#if Q_WS_WIN
#include <vmime/platforms/windows/windowsHandler.hpp>
#else
#include <vmime/platforms/posix/posixHandler.hpp>
#endif

#include <util/resourceloader.h>
#include "message.h"
#include "storage.h"
#include "progressmanager.h"

namespace LeechCraft
{
namespace Snails
{
	Core::Core ()
	: AccountsModel_ (new QStandardItemModel)
	, Storage_ (new Storage (this))
	, ProgressManager_ (new ProgressManager (this))
	, MsgView_ (new Util::ResourceLoader ("snails/msgview"))
	{
#if Q_WS_WIN
		vmime::platform::setHandler<vmime::platforms::windows::windowsHandler> ();
#else
		vmime::platform::setHandler<vmime::platforms::posix::posixHandler> ();
#endif

		MsgView_->AddGlobalPrefix ();
		MsgView_->AddLocalPrefix ();

		qRegisterMetaType<Message_ptr> ("LeechCraft::Snails::Message_ptr");
		qRegisterMetaType<Message_ptr> ("Message_ptr");
		qRegisterMetaType<QList<Message_ptr>> ("QList<LeechCraft::Snails::Message_ptr>");
		qRegisterMetaType<QList<Message_ptr>> ("QList<Message_ptr>");
		qRegisterMetaType<ProgressListener_g_ptr> ("ProgressListener_g_ptr");
		qRegisterMetaType<Account::FetchFlags> ("Account::FetchFlags");

		QStringList headers;
		headers << tr ("Name")
				<< tr ("Server")
				<< tr ("Type");
		AccountsModel_->setHorizontalHeaderLabels (headers);

		LoadAccounts ();
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::Release ()
	{
		MsgView_.reset ();
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	void Core::SendEntity (const Entity& e)
	{
		emit gotEntity (e);
	}

	QAbstractItemModel* Core::GetAccountsModel () const
	{
		return AccountsModel_;
	}

	QList<Account_ptr> Core::GetAccounts () const
	{
		return Accounts_;
	}

	Account_ptr Core::GetAccount (const QModelIndex& index) const
	{
		if (!index.isValid ())
			return Account_ptr ();

		return Accounts_ [index.row ()];
	}

	Storage* Core::GetStorage () const
	{
		return Storage_;
	}

	ProgressManager* Core::GetProgressManager () const
	{
		return ProgressManager_;
	}

	QString Core::GetMsgViewTemplate () const
	{
		auto dev = MsgView_->Load ("default/msgview.html");
		if (!dev || !dev->open (QIODevice::ReadOnly))
			return tr ("<h1 style='color:#f00'>Unable to load message view template.</h1>");

		return QString::fromUtf8 (dev->readAll ());
	}

	void Core::AddAccount (Account_ptr account)
	{
		AddAccountImpl (account);

		saveAccounts ();
	}

	void Core::AddAccountImpl (Account_ptr account)
	{
		Accounts_ << account;

		QList<QStandardItem*> row;
		row << new QStandardItem (account->GetName ());
		row << new QStandardItem (account->GetServer ());
		row << new QStandardItem (account->GetType ());
		AccountsModel_->appendRow (row);

		ProgressManager_->AddAccount (account.get ());

		connect (account.get (),
				SIGNAL (accountChanged ()),
				this,
				SLOT (saveAccounts ()));
	}

	void Core::LoadAccounts ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Snails_Accounts");
		Q_FOREACH (const QVariant& var, settings.value ("Accounts").toList ())
		{
			Account_ptr acc (new Account);
			try
			{
				acc->Deserialize (var.toByteArray ());
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to deserialize account, sorry :("
						<< e.what ();
				continue;
			}
			AddAccountImpl (acc);
		}
	}

	void Core::saveAccounts () const
	{
		QList<QVariant> serialized;
		Q_FOREACH (Account_ptr acc, Accounts_)
			serialized << acc->Serialize ();

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Snails_Accounts");
		settings.setValue ("Accounts", serialized);
	}
}
}
