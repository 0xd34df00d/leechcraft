/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "localblogaccount.h"
#include <stdexcept>
#include <QtDebug>
#include "accountconfigurationdialog.h"
#include "accountconfigurationwidget.h"
#include "accountstorage.h"
#include "localbloggingplatform.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Blogique
{
namespace Hestia
{
	constexpr auto DefaultPostsCount = 20;

	LocalBlogAccount::LocalBlogAccount (const QString& name, QObject *parent)
	: QObject (parent)
	, ParentBloggingPlatform_ (qobject_cast<LocalBloggingPlatform*> (parent))
	, Name_ (name)
	, AccountStorage_ (new AccountStorage (this))
	, LoadAllEvents_ (new QAction (tr ("All entries"), this))
	{
		connect (LoadAllEvents_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleLoadAllEvents ()));
	}

	QObject* LocalBlogAccount::GetQObject ()
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

	void LocalBlogAccount::RenameAccount (const QString&)
	{
	}

	QByteArray LocalBlogAccount::GetAccountID () const
	{
		return ParentBloggingPlatform_->GetBloggingPlatformID () + "_" +
				Name_.toUtf8 ();
	}

	void LocalBlogAccount::OpenConfigurationDialog ()
	{
		AccountConfigurationDialog dia;

		if (!DatabasePath_.isEmpty ())
			dia.ConfWidget ()->SetAccountBasePath (DatabasePath_);

		if (dia.exec () == QDialog::Rejected)
			return;

		FillSettings (dia.ConfWidget ());
	}

	bool LocalBlogAccount::IsValid () const
	{
		return IsValid_;
	}

	QObject* LocalBlogAccount::GetProfile ()
	{
		return nullptr;
	}

	void LocalBlogAccount::RemoveEntry (const Entry& entry)
	{
		try
		{
			AccountStorage_->RemoveEntry (entry.EntryId_);
			emit entryRemoved (entry.EntryId_);
			emit requestEntriesBegin ();
			handleLoadAllEvents ();
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}
	}

	void LocalBlogAccount::UpdateEntry (const Entry& entry)
	{
		try
		{
			AccountStorage_->UpdateEntry (entry, entry.EntryId_);
			emit entryUpdated ({ entry });
			emit requestEntriesBegin ();
			handleLoadAllEvents ();
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}
	}

	QList<QAction*> LocalBlogAccount::GetUpdateActions () const
	{
		return { LoadAllEvents_ };
	}

	void LocalBlogAccount::RequestLastEntries (int count)
	{
		try
		{
			emit requestEntriesBegin ();
			emit gotEntries (AccountStorage_->GetLastEntries (AccountStorage::Mode::FullMode,
					count ? count : DefaultPostsCount));
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}
	}

	void LocalBlogAccount::RequestStatistics ()
	{
		try
		{
			emit gotBlogStatistics (AccountStorage_->GetEntriesCountByDate ());
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}
	}

	void LocalBlogAccount::RequestTags ()
	{
		emit tagsUpdated (GetTags ());
	}

	void LocalBlogAccount::GetEntriesByDate (const QDate& date)
	{
		try
		{
			emit gotEntries (AccountStorage_->GetEntriesByDate (date));
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}
	}

	void LocalBlogAccount::GetEntriesWithFilter (const Filter& filter)
	{
		try
		{
			emit gotFilteredEntries (AccountStorage_->GetEntriesWithFilter (filter));
			emit gettingFilteredEntriesFinished ();
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}
	}

	void LocalBlogAccount::RequestRecentComments ()
	{
	}

	void LocalBlogAccount::AddComment (const CommentEntry&)
	{
	}

	void LocalBlogAccount::DeleteComment (qint64, bool)
	{
	}

	QHash<QString, int> LocalBlogAccount::GetTags () const
	{
		try
		{
			return AccountStorage_->GetAllTags ();
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
			return QHash<QString, int> ();
		}
	}

	void LocalBlogAccount::FillSettings (AccountConfigurationWidget *widget)
	{
		DatabasePath_ = widget->GetAccountBasePath ();
		if (DatabasePath_.isEmpty ())
			return;

		if (widget->GetOption () & IBloggingPlatform::AAORegisterNewAccount)
		{
			IsValid_ = true;
			AccountStorage_->Init (DatabasePath_);
			emit accountValidated (IsValid_);
		}
		else
			Validate ();
	}

	void LocalBlogAccount::Init ()
	{
		if (IsValid_)
			AccountStorage_->Init (DatabasePath_);
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
			return nullptr;
		}

		QString name;
		in >> name;
		auto result = new LocalBlogAccount (name, parent);
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

	void LocalBlogAccount::submit (const Entry& e)
	{
		try
		{
			AccountStorage_->SaveNewEntry (e);
			emit entryPosted ({ e });
			emit requestEntriesBegin ();
			handleLoadAllEvents ();
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}
	}

	void LocalBlogAccount::preview (const Entry&)
	{
	}

	void LocalBlogAccount::handleLoadAllEvents ()
	{
		try
		{
			emit gotEntries (AccountStorage_->GetEntries (AccountStorage::Mode::FullMode));
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}
	}

}
}
}
