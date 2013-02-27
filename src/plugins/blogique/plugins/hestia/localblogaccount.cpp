/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "localblogaccount.h"
#include <QtDebug>
#include "accountstorage.h"
#include "importaccountwidget.h"
#include "localbloggingplatform.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Hestia
{
	LocalBlogAccount::LocalBlogAccount (const QString& name, QObject *parent)
	: QObject (parent)
	, ParentBloggingPlatform_ (qobject_cast<LocalBloggingPlatform*> (parent))
	, Name_ (name)
	, IsValid_ (false)
	, AccountStorage_ (new AccountStorage (this))
	{
	}

	QObject* LocalBlogAccount::GetObject ()
	{
		return this;
	}

	QObject* LocalBlogAccount::GetParentBloggingPlatform () const
	{
		return ParentBloggingPlatform_;
	}

	QString LocalBlogAccount::GetAccountName () const
	{
		return Name_;
	}

	QString LocalBlogAccount::GetOurLogin () const
	{
		return QString ();
	}

	void LocalBlogAccount::RenameAccount (const QString& name)
	{

	}

	QByteArray LocalBlogAccount::GetAccountID () const
	{
		return ParentBloggingPlatform_->GetBloggingPlatformID () + "_" +
				Name_.toUtf8 ();
	}

	void LocalBlogAccount::OpenConfigurationDialog ()
	{

	}

	bool LocalBlogAccount::IsValid () const
	{
		return IsValid_;
	}

	QObject* LocalBlogAccount::GetProfile ()
	{
		return 0;
	}

	void LocalBlogAccount::RemoveEntry (const LeechCraft::Blogique::Entry& entry)
	{
	}

	void LocalBlogAccount::UpdateEntry (const LeechCraft::Blogique::Entry& entry)
	{

	}

	QList<QAction*> LocalBlogAccount::GetUpdateActions () const
	{
		return QList<QAction*> ();
	}

	void LocalBlogAccount::RequestStatistics ()
	{
	}


	void LocalBlogAccount::GetEntriesByDate (const QDate& date)
	{
	}

	void LocalBlogAccount::GetLastEntries (int count)
	{
	}

	void LocalBlogAccount::FillSettings (ImportAccountWidget *widget)
	{
		DatabasePath_ = widget->GetAccountBasePath ();
		Validate ();
	}

	void LocalBlogAccount::Init ()
	{
		connect (this,
				SIGNAL (accountValidated (bool)),
				ParentBloggingPlatform_,
				SLOT (handleAccountValidated (bool)));

		connect (this,
				SIGNAL (accountSettingsChanged ()),
				ParentBloggingPlatform_,
				SLOT (saveAccounts ()));
	}

	QByteArray LocalBlogAccount::Serialize () const
	{
		quint16 ver = 1;
		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << ver
					<< Name_
					<< DatabasePath_
					<< IsValid_;
		}

		return result;
	}

	LocalBlogAccount* LocalBlogAccount::Deserialize (const QByteArray& data, QObject *parent)
	{
		quint16 ver = 0;
		QDataStream in (data);
		in >> ver;

		if (ver != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< ver;
			return 0;
		}

		QString name;
		in >> name;
		LocalBlogAccount *result = new LocalBlogAccount (name, parent);
		in >> result->DatabasePath_
				>> result->IsValid_;

		return result;
	}

	void LocalBlogAccount::Validate ()
	{
		IsValid_ = AccountStorage_->CheckDatabase (DatabasePath_);
		emit accountValidated (IsValid_);
	}

	void LocalBlogAccount::updateProfile ()
	{

	}

	void LocalBlogAccount::submit (const Entry& event)
	{

	}

	void LocalBlogAccount::backup ()
	{

	}

}
}
}