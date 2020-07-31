/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "logger.h"
#include <QDir>
#include <QSqlError>
#include <util/db/oral/oral.h>
#include <util/db/util.h>
#include <util/sll/unreachable.h>
#include <util/sys/paths.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iaccount.h>

namespace LC
{
namespace Azoth
{
namespace Herbicide
{
	struct Logger::AccountRecord
	{
		Util::oral::PKey<int> PKey_;

		Util::oral::Unique<QString> AccountID_;
		QString AccountName_;

		static QByteArray ClassName ()
		{
			return "AccountRecord";
		}
	};

	struct Logger::EntryRecord
	{
		Util::oral::PKey<int> PKey_;
		Util::oral::References<&AccountRecord::PKey_> AccountID_;

		Util::oral::Unique<QString> EntryID_;
		QString EntryHumanReadableId_;
		QString EntryName_;

		static QByteArray ClassName ()
		{
			return "EntryRecord";
		}
	};

	struct Logger::EventRecord
	{
		Util::oral::PKey<int> PKey_;
		Util::oral::References<&EntryRecord::PKey_> EntryID_;

		Logger::Event Event_;
		QString Reason_;

		static QByteArray ClassName ()
		{
			return "EventRecord";
		}
	};
}
}
}

BOOST_FUSION_ADAPT_STRUCT (LC::Azoth::Herbicide::Logger::AccountRecord,
		PKey_,
		AccountID_,
		AccountName_)

BOOST_FUSION_ADAPT_STRUCT (LC::Azoth::Herbicide::Logger::EntryRecord,
		PKey_,
		AccountID_,
		EntryID_,
		EntryHumanReadableId_,
		EntryName_)

BOOST_FUSION_ADAPT_STRUCT (LC::Azoth::Herbicide::Logger::EventRecord,
		PKey_,
		EntryID_,
		Event_,
		Reason_)

namespace LC
{
namespace Util
{
namespace oral
{
	template<typename ImplFactory>
	struct Type2Name<ImplFactory, Azoth::Herbicide::Logger::Event>
	{
		auto operator() () const
		{
			return Type2Name<ImplFactory, QString> {} ();
		}
	};

	template<>
	struct ToVariant<Azoth::Herbicide::Logger::Event>
	{
		QVariant operator() (Azoth::Herbicide::Logger::Event event) const
		{
			switch (event)
			{
			case Azoth::Herbicide::Logger::Event::Granted:
				return "granted";
			case Azoth::Herbicide::Logger::Event::Denied:
				return "denied";
			case Azoth::Herbicide::Logger::Event::Challenged:
				return "challenged";
			case Azoth::Herbicide::Logger::Event::Succeeded:
				return "succeeded";
			case Azoth::Herbicide::Logger::Event::Failed:
				return "failed";
			}

			Util::Unreachable ();
		}
	};

	template<>
	struct FromVariant<Azoth::Herbicide::Logger::Event>
	{
		Azoth::Herbicide::Logger::Event operator() (const QVariant& var) const
		{
			static const QMap<QString, Azoth::Herbicide::Logger::Event> map
			{
				{ "granted", Azoth::Herbicide::Logger::Event::Granted },
				{ "denied", Azoth::Herbicide::Logger::Event::Denied },
				{ "challenged", Azoth::Herbicide::Logger::Event::Challenged },
				{ "succeeded", Azoth::Herbicide::Logger::Event::Succeeded },
				{ "failed", Azoth::Herbicide::Logger::Event::Failed },
			};
			return map.value (var.toString ());
		}
	};
}
}

namespace Azoth
{
namespace Herbicide
{
	Logger::Logger (QObject *parent)
	: QObject { parent }
	, DB_ { QSqlDatabase::addDatabase ("QSQLITE",
				Util::GenConnectionName ("org.LeechCraft.Azoth.Herbicide.Log")) }
	{
		const auto& herbicideDir = Util::GetUserDir (Util::UserDir::LC, "azoth/herbicide");
		DB_.setDatabaseName (herbicideDir.filePath ("log.db"));
		if (!DB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open the database";
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error { "Cannot create database" };
		}

		Util::RunTextQuery (DB_, "PRAGMA synchronous = NORMAL;");
		Util::RunTextQuery (DB_, "PRAGMA journal_mode = WAL;");

		AdaptedAccount_ = Util::oral::AdaptPtr<AccountRecord> (DB_);
		AdaptedEntry_ = Util::oral::AdaptPtr<EntryRecord> (DB_);
		AdaptedEvent_ = Util::oral::AdaptPtr<EventRecord> (DB_);
	}

	namespace sph = Util::oral::sph;

	void Logger::LogEvent (Logger::Event event, const ICLEntry *entry, const QString& descr)
	{
		const QString accId { entry->GetParentAccount ()->GetAccountID () };
		const auto& entryId = entry->GetEntryID ();

		const auto& maybeAccPKey = AdaptedAccount_->SelectOne (sph::fields<&AccountRecord::PKey_>,
				sph::f<&AccountRecord::AccountID_> == accId);
		const auto accPKey = maybeAccPKey ?
				*maybeAccPKey :
				InsertAccount (entry->GetParentAccount ());

		const auto maybeEntryPKey = AdaptedEntry_->SelectOne (sph::fields<&EntryRecord::PKey_>,
				sph::f<&AccountRecord::AccountID_> == entryId);
		const auto entryPKey = maybeEntryPKey ?
				*maybeEntryPKey :
				InsertEntry (accPKey, entry);

		AdaptedEvent_->Insert ({ {}, entryPKey, event, descr });
	}

	int Logger::InsertAccount (const IAccount *acc)
	{
		return AdaptedAccount_->Insert ({
				{},
				QString { acc->GetAccountID () },
				acc->GetAccountName ()
			});
	}

	int Logger::InsertEntry (int accPKey, const ICLEntry *entry)
	{
		return AdaptedEntry_->Insert ({
				{},
				accPKey,
				entry->GetEntryID (),
				entry->GetHumanReadableID (),
				entry->GetEntryName ()
			},
			Util::oral::InsertAction::Replace::Fields<&EntryRecord::EntryID_>);
	}
}
}
}
