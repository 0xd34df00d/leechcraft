/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "storage2.h"
#include <QDir>
#include <QSqlError>
#include <util/db/oral/oral.h>
#include <util/sll/visitor.h>
#include <util/sys/paths.h>

namespace LC::Util::oral
{
	template<typename ImplFactory>
	struct Type2Name<ImplFactory, Azoth::IMessage::Direction>
	{
		constexpr auto operator() () const noexcept
		{
			return "VARCHAR(1)"_ct;
		}
	};

	template<>
	struct ConvertT<Azoth::IMessage::Direction>
	{
		QVariant operator() (Azoth::IMessage::Direction kind) const noexcept
		{
			using enum Azoth::IMessage::Direction;
			switch (kind)
			{
			case In:
				return QString { QChar { 'I' } };
			case Out:
				return QString { QChar { 'O' } };
			}
			std::unreachable ();
		}

		Azoth::IMessage::Direction operator() (const QVariant& var) const noexcept
		{
			using enum Azoth::IMessage::Direction;

			if (const auto& str = var.toString ();
				str.size () == 1)
				switch (str.at (0).toLatin1 ())
				{
				case 'I':
					return In;
				case 'O':
					return Out;
				}

			qWarning () << "invalid stored value:" << var;
			return In;
		}
	};

	template<typename ImplFactory>
	struct Type2Name<ImplFactory, Azoth::History::EntryKind>
	{
		constexpr auto operator() () const noexcept
		{
			return "VARCHAR(1)"_ct;
		}
	};

	template<>
	struct ConvertT<Azoth::History::EntryKind>
	{
		QVariant operator() (Azoth::History::EntryKind kind) const noexcept
		{
			using enum Azoth::History::EntryKind;
			switch (kind)
			{
			case Chat:
				return QString { QChar { 'C' } };
			case MUC:
				return QString { QChar { 'M' } };
			case PrivateChat:
				return QString { QChar { 'P' } };
			}
			std::unreachable ();
		}

		Azoth::History::EntryKind operator() (const QVariant& var) const noexcept
		{
			using enum Azoth::History::EntryKind;

			if (const auto& str = var.toString ();
				str.size () == 1)
				switch (str.at (0).toLatin1 ())
				{
				case 'C':
					return Chat;
				case 'M':
					return MUC;
				case 'P':
					return PrivateChat;
				}

			qWarning () << "invalid stored value:" << var;
			return Chat;
		}
	};
}

namespace LC::Azoth::ChatHistory
{
	namespace o = Util::oral;

	struct Storage2::AccountRecord
	{
		o::PKey<qint64> Id_ {};

		o::Unique<o::NotNull<QByteArray>> AccountId_;
		o::NotNull<QString> AccountName_;

		AccountInfo ToInfo () const;

		constexpr static auto ClassName = "Account"_ct;
	};

	struct Storage2::EntryRecord
	{
		o::PKey<qint64> Id_ {};

		o::References<&AccountRecord::Id_> Account_;

		o::NotNull<QString> HumanReadableId_;
		o::NotNull<History::EntryKind> Kind_;

		// Roster entries ↦ contact name
		// MUC entries ↦ MUC name
		// MUC participant ↦ entries: nickname
		o::NotNull<QString> DisplayName_;

		constexpr static auto ClassName = "Entry"_ct;

		using Constraints = o::Constraints<
				o::UniqueSubset<&EntryRecord::Account_, &EntryRecord::HumanReadableId_>
			>;
	};

	// Additional information for a MUC room or a particular MUC participant.
	struct Storage2::MucContextRecord
	{
		o::PKey<o::References<&EntryRecord::Id_>> Id_ {};

		o::NotNull<QString> MucName_;
		std::optional<QByteArray> ParticipantPersistentId_ {};	// Private chat entries: XEP-0421-like persistent ID.

		constexpr static auto ClassName = "MucContext"_ct;
	};

	struct Storage2::MessageRecord
	{
		o::PKey<qint64> Id_ {};
		o::References<&EntryRecord::Id_> Entry_;

		// "Sender entry name/variant" below means that if the message is outgoing, it's _our_ name and variant.
		o::NotNull<QString> DisplayName_;			// Roster messages: sender entry name; MUC and private messages: party nickname.
		std::optional<QString> Variant_;			// Roster messages: sender entry variant (if supported by the protocol).
		std::optional<QByteArray> PersistentId_;	// MUC public messages: XEP-0421-like persistent ID.

		o::NotNull<QDateTime> TS_;
		o::NotNull<IMessage::Direction> Direction_;
		o::NotNull<QString> Body_;
		std::optional<QString> RichBody_;

		constexpr static auto ClassName = "Message"_ct;

		using Indices = o::Indices<
				o::Index<&MessageRecord::Entry_, &MessageRecord::TS_>
			>;
	};
}

ORAL_ADAPT_STRUCT (LC::Azoth::ChatHistory::Storage2::AccountRecord
		, Id_
		, AccountId_
		, AccountName_
		);

ORAL_ADAPT_STRUCT (LC::Azoth::ChatHistory::Storage2::EntryRecord
		, Id_
		, Account_
		, HumanReadableId_
		, Kind_
		, DisplayName_
		);

ORAL_ADAPT_STRUCT (LC::Azoth::ChatHistory::Storage2::MucContextRecord
		, Id_
		, MucName_
		, ParticipantPersistentId_
		);

ORAL_ADAPT_STRUCT (LC::Azoth::ChatHistory::Storage2::MessageRecord
		, Id_
		, Entry_
		, DisplayName_
		, Variant_
		, PersistentId_
		, TS_
		, Direction_
		, Body_
		, RichBody_
		);

namespace LC::Azoth::ChatHistory
{
	AccountInfo Storage2::AccountRecord::ToInfo () const
	{
		return
		{
			.Id_ = Id_,
			.AccountId_ = *AccountId_,
			.AccountName_ =	AccountName_,
		};
	}

	namespace sph = Util::oral::sph;

	Storage2::Storage2 (QObject *parent)
	: QObject { parent }
	, DB_ { QSqlDatabase::addDatabase ("QSQLITE", Util::GenConnectionName ("org.LC.Azoth.ChatHistory.Storage2")) }
	{
		DB_.setDatabaseName (Util::CreateIfNotExists ("azoth").filePath ("history2.db"));

		if (!DB_.open ())
		{
			qCritical () << "cannot open the database";
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error { "Azoth ChatHistory: cannot create v2 database" };
		}

		Util::RunTextQuery (DB_, "PRAGMA foreign_keys = ON;");
		Util::RunTextQuery (DB_, "PRAGMA synchronous = NORMAL;");
		Util::RunTextQuery (DB_, "PRAGMA journal_mode = WAL;");
		Util::RunTextQuery (DB_, "PRAGMA case_sensitive_like = ON;");

		Accounts_ = o::AdaptPtr<AccountRecord> (DB_);
		Entries_ = o::AdaptPtr<EntryRecord> (DB_);
		MucContexts_ = o::AdaptPtr<MucContextRecord> (DB_);
		Messages_ = o::AdaptPtr<MessageRecord> (DB_);
	}

	Storage2::~Storage2 () = default;

	std::optional<QDateTime> Storage2::GetLastTimestamp (const QByteArray& accountId) const
	{
		return Messages_->Select (sph::max<&MessageRecord::TS_>,
				sph::f<&MessageRecord::Entry_> == sph::f<&EntryRecord::Id_> &&
				sph::f<&EntryRecord::Account_> == sph::f<&AccountRecord::Id_> &&
				sph::f<&AccountRecord::AccountId_> == accountId);
	}

	QList<AccountInfo> Storage2::GetAccounts () const
	{
		return Util::Map (Accounts_->Select (), &AccountRecord::ToInfo);
	}

	namespace
	{
		QHash<qint64, QString> BuildEntryId2MucName (const QList<Storage2::MucContextRecord>& mucContexts)
		{
			QHash<qint64, QString> result;
			result.reserve (mucContexts.size ());
			for (const auto& record : mucContexts)
				result [*record.Id_] = record.MucName_;
			return result;
		}

		History::SomeEntryDescr MakeSpecificInfo (const Storage2::EntryRecord& entry,
				const QHash<qint64, QString>& mucNames)
		{
			using enum History::EntryKind;
			switch (entry.Kind_)
			{
			case Chat:
				return History::EntryDescr<Chat> { .Nick_ = entry.DisplayName_ };
			case MUC:
				return History::EntryDescr<MUC> { .MucName_ = entry.DisplayName_ };
			case PrivateChat:
				// TODO XEP-0421 persistent IDs
				return History::EntryDescr<PrivateChat> { .MucName_ = mucNames [entry.Id_], .Nick_ = entry.DisplayName_, .PersistentId_ = {} };
			}
			qFatal () << "unknown entry kind" << static_cast<int> (*entry.Kind_);
			std::unreachable ();
		}
	}

	QList<Entry> Storage2::GetEntries (qint64 accountId) const
	{
		const auto& records = Entries_->Select (sph::f<&EntryRecord::Account_> == accountId);

		const auto& mucContexts = MucContexts_->Select (sph::f<&MucContextRecord::Id_> == sph::f<&EntryRecord::Id_> &&
				sph::f<&EntryRecord::Account_> == accountId);
		const auto& mucNamesDict = BuildEntryId2MucName (mucContexts);

		QList<Entry> result;
		result.reserve (records.size ());
		for (const auto& record : records)
			result << Entry
			{
				.Id_ = record.Id_,
				.HumanReadableId_ = record.HumanReadableId_,
				.EntryInfo_ = MakeSpecificInfo (record, mucNamesDict),
			};
		return result;
	}

	namespace
	{
		Storage2::HistoryMessage ToHistoryMessage (const Storage2::MessageRecord& msg)
		{
			return
			{
				.Id_ = msg.Id_,
				.DisplayName_ = msg.DisplayName_,
				.Variant_ = msg.Variant_,
				.Direction_ = msg.Direction_,
				.TS_ = msg.TS_,
				.Body_ = msg.Body_,
				.RichBody_ = msg.RichBody_,
			};
		}
	}

	Storage2::Cursor Storage2::Cursor::FromMessage (const HistoryMessage& msg)
	{
		return { .TS_ = msg.TS_, .MsgId_ = msg.Id_ };
	}

	Storage2::Cursor Storage2::Cursor::Min ()
	{
		const QDateTime sentinel { QDate { 1000, 1, 1 }, QTime { 0, 0, 0 } };
		return { sentinel, -1 };
	}

	Storage2::Cursor Storage2::Cursor::Max ()
	{
		const QDateTime sentinel { QDate { 9999, 12, 31 }, QTime { 23, 59, 59 } };
		return { sentinel, std::numeric_limits<qint64>::max () };
	}

	namespace
	{
		auto AsTuple (const Storage2::Cursor& cursor)
		{
			return std::tuple { cursor.TS_, cursor.MsgId_ };
		}
	}

	QList<Storage2::HistoryMessage> Storage2::GetMessages (const Entry& entry, const Pagination& pagination) const
	{
		const auto cursor = pagination.Cursor_.value_or (Cursor::Max ());

		QList<MessageRecord> records;
		if (pagination.Before_)
		{
			auto preceding = Messages_->Select.Build ()
				.Where (sph::f<&MessageRecord::Entry_> == entry.Id_ &&
						sph::tuple<&MessageRecord::TS_, &MessageRecord::Id_> < AsTuple (cursor))
				.Order (o::OrderBy<sph::desc<&MessageRecord::TS_, &MessageRecord::Id_>>)
				.Limit (pagination.Before_)
				();
			std::ranges::reverse (preceding);
			records += std::move (preceding);
		}
		if (pagination.IncludeCursor_)
			records += Messages_->Select (sph::f<&MessageRecord::Id_> == cursor.MsgId_);
		if (pagination.After_)
			records += Messages_->Select.Build ()
				.Where (sph::f<&MessageRecord::Entry_> == entry.Id_ &&
						sph::tuple<&MessageRecord::TS_, &MessageRecord::Id_> > AsTuple (cursor))
				.Order (o::OrderBy<sph::asc<&MessageRecord::TS_, &MessageRecord::Id_>>)
				.Limit (pagination.After_)
				();

		return Util::Map (std::move (records), &ToHistoryMessage);
	}

	QList<Storage2::HistoryMessage> Storage2::GetMessagesDated (const Entry& entry, const QDate& date) const
	{
		auto records = Messages_->Select.Build ()
				.Where (sph::f<&MessageRecord::Entry_> == entry.Id_ &&
						sph::f<&MessageRecord::TS_> >= QDateTime { date, {} } &&
						sph::f<&MessageRecord::TS_> <  QDateTime { date.addDays (1), {} })
				.Order (o::OrderBy<sph::asc<&MessageRecord::TS_, &MessageRecord::Id_>>)
				();
		return Util::Map (std::move (records), &ToHistoryMessage);
	}

	QList<Storage2::HistoryMessage> Storage2::GetLastMessages (const QByteArray& accountId,
			const QString& entryHumanReadableId, size_t count) const
	{
		const auto entryId = Entries_->SelectOne (sph::fields<&EntryRecord::Id_>,
				sph::f<&EntryRecord::Account_> == sph::f<&AccountRecord::Id_> &&
				sph::f<&AccountRecord::AccountId_> == accountId &&
				sph::f<&EntryRecord::HumanReadableId_> == entryHumanReadableId);
		if (!entryId)
			return {};

		auto messages = Messages_->Select.Build ()
				.Where (sph::f<&MessageRecord::Entry_> == entryId)
				.Order (o::OrderBy<sph::desc<&MessageRecord::TS_, &MessageRecord::Id_>>)
				.Limit (count)
				();
		std::ranges::reverse (messages);
		return Util::Map (std::move (messages), &ToHistoryMessage);
	}

	QList<int> Storage2::GetDaysWithHistory (const Entry& entry, int year, int month) const
	{
		const QDate start { year, month, 1 };
		return Messages_->Select (sph::distinct { sph::fun<int, "CAST(strftime('%d', @) AS INTEGER)"_ct, &MessageRecord::TS_> },
				sph::f<&MessageRecord::Entry_> == entry.Id_ &&
				sph::f<&MessageRecord::TS_> >= QDateTime { start, {} } &&
				sph::f<&MessageRecord::TS_> <  QDateTime { start.addMonths (1), {} });
	}

	namespace
	{
		template<typename T, typename F, typename... Fs>
		auto Pipe (T&& arg, F&& f, Fs&&... fs)
		{
			if constexpr (!sizeof... (Fs))
				return std::invoke (std::forward<F> (f), std::forward<T> (arg));
			else
			{
				auto cont = [...fs = std::forward<Fs> (fs)]<typename TT> (TT&& ret)
				{
					return Pipe (std::forward<TT> (ret), std::forward<Fs> (fs)...);
				};
				return std::invoke (std::forward<F> (f), std::move (cont), std::forward<T> (arg));
			}
		}
	}

	std::optional<Storage2::Cursor> Storage2::Search (const Entry& entry,
			const QString& text, Qt::CaseSensitivity cs, SearchDirection dir,
			const Cursor& from) const
	{
		using namespace o::infix;

		auto srcQuery = Messages_->SelectOne.Build ()
				.Select (sph::fields<&MessageRecord::TS_, &MessageRecord::Id_>)
				.Where (sph::f<&MessageRecord::Entry_> == entry.Id_)
				.Limit (1)
				;

		const auto withText = [cs, &text] (const auto& cont, auto&& query)
		{
			const auto& pattern = '%' + text + '%';
			switch (cs)
			{
			case Qt::CaseSensitive:
				return cont (std::move (query).AndWhere (sph::f<&MessageRecord::Body_> |like| pattern));
			case Qt::CaseInsensitive:
				return cont (std::move (query).AndWhere (sph::f<&MessageRecord::Body_> |ilike| pattern));
			}
			std::unreachable ();
		};
		const auto withBound = [dir, from] (const auto& cont, auto&& query)
		{
			switch (dir)
			{
			case SearchDirection::Backward:
				return cont (std::move (query)
						.AndWhere (sph::tuple<&MessageRecord::TS_, &MessageRecord::Id_> < AsTuple (from))
						.Order (o::OrderBy<sph::desc<&MessageRecord::TS_, &MessageRecord::Id_>>));
			case SearchDirection::Forward:
				return cont (std::move (query)
						.AndWhere (sph::tuple<&MessageRecord::TS_, &MessageRecord::Id_> > AsTuple (from))
						.Order (o::OrderBy<sph::asc<&MessageRecord::TS_, &MessageRecord::Id_>>));
			}
			std::unreachable ();
		};
		const auto execute = [] (auto&& query) { return std::move (query) (); };

		return Pipe (srcQuery, withText, withBound, execute)
				.transform ([&] (const auto& fields)
						{
							const auto& [ts, id] = fields;
							return Cursor { .TS_ = ts, .MsgId_ = id };
						});
	}

	namespace
	{
		template<History::EntryKind Kind>
		QString GetDisplayName (const History::EntryDescr<Kind>& descr)
		{
			if constexpr (Kind == History::EntryKind::Chat)
				return descr.Nick_;
			if constexpr (Kind == History::EntryKind::MUC)
				return descr.MucName_;
			if constexpr (Kind == History::EntryKind::PrivateChat)
				return descr.Nick_;
			std::unreachable ();
		}
	}

	void Storage2::AddMessage (const History::SomeEntryWithMessages& some)
	{
		Util::DBLock lock { DB_ };
		lock.Init ();

		using namespace History;

		Util::Visit (some,
				[this]<EntryKind Kind> (const EntryWithMessages<Kind>& entryWithMessages)
				{
					const auto& [entry, messages] = entryWithMessages;
					if (messages.isEmpty ())
						return;
					const auto accountId = Accounts_->Insert ({
								.AccountId_ = entry.AccountId_,
								.AccountName_ = entry.AccountName_,
							},
							o::InsertAction::Replace::Fields<&AccountRecord::AccountName_>);
					const auto entryId = Entries_->Insert ({
								.Account_ = accountId,
								.HumanReadableId_ = entry.EntryHumanReadableId_,
								.Kind_ = Kind,
								.DisplayName_ = GetDisplayName (entry.Description_),
							},
							o::InsertAction::Replace::Fields<&EntryRecord::DisplayName_>);

					if constexpr (Kind == EntryKind::MUC)
						MucContexts_->Insert ({
									.Id_ = entryId,
									.MucName_ = entry.Description_.MucName_,
								},
								o::InsertAction::Replace::Fields<&MucContextRecord::MucName_>);
					if constexpr (Kind == EntryKind::PrivateChat)
						MucContexts_->Insert ({
									.Id_ = entryId,
									.MucName_ = entry.Description_.MucName_,
									.ParticipantPersistentId_ = entry.Description_.PersistentId_,
								},
								o::InsertAction::Replace::Fields<&MucContextRecord::MucName_>);

					for (const auto& msg : messages)
					{
						const auto direction = std::holds_alternative<SelfEndpoint> (msg.Endpoint_) ?
								IMessage::Direction::Out :
								IMessage::Direction::In;

						const auto [displayName, variant] = Util::Visit (msg.Endpoint_,
								[] (const SelfEndpoint& self) { return std::pair { self.Name_, self.Variant_} ; },
								[&] (const EntryEndpoint<Kind>& endpoint) -> std::pair<QString, std::optional<QString>>
								{
									if constexpr (Kind == EntryKind::Chat)
										return { entry.Description_.Nick_, endpoint.Variant_ };
									if constexpr (Kind == EntryKind::MUC)
										return { endpoint.Nick_, {} };
									if constexpr (Kind == EntryKind::PrivateChat)
										return { entry.Description_.Nick_, {} };
								});

						std::optional<QByteArray> persistentId;
						if constexpr (Kind == EntryKind::MUC)
							if (const auto endpoint = std::get_if<EntryEndpoint<EntryKind::MUC>> (&msg.Endpoint_))
								persistentId = endpoint->PersistentId_;

						Messages_->Insert ({
								.Entry_ = entryId,
								.DisplayName_ = displayName,
								.Variant_ = variant,
								.PersistentId_ = persistentId,
								.TS_ = msg.TS_,
								.Direction_ = direction,
								.Body_ = msg.Body_,
								.RichBody_ = msg.RichBody_,
							});
					}
				});

		lock.Good ();
	}

	void Storage2::ClearHistory (const Entry& entry)
	{
		Util::DBLock lock { DB_ };
		lock.Init ();
		Entries_->DeleteBy (sph::f<&EntryRecord::Id_> == entry.Id_);
		lock.Good ();
	}
}
