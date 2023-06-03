/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sqlstoragebackend.h"
#include <stdexcept>
#include <cmath>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QtDebug>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include <util/db/oral/oral.h>
#include <util/db/oral/pgimpl.h>
#include <util/sll/util.h>
#include <util/sll/qtutil.h>
#include <util/util.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Poshuku
{
	namespace oral = Util::oral;
	namespace sph = Util::oral::sph;

	struct SQLStorageBackend::History
	{
		oral::PKey<QDateTime, oral::NoAutogen> Date_;
		QString Title_;
		QString URL_;

		constexpr static auto ClassName = "History"_ct;

		using Indices = oral::Indices<
				oral::Index<&History::Title_, &History::URL_>
			>;

		HistoryItem ToHistoryItem () const
		{
			return { Title_, *Date_, URL_ };
		}

		static History FromHistoryItem (const HistoryItem& item)
		{
			return
			{
				item.DateTime_,
				item.Title_,
				item.URL_
			};
		}
	};

	struct SQLStorageBackend::Favorites
	{
		oral::PKey<QString, oral::NoAutogen> Title_;
		QString URL_;
		QString Tags_;

		constexpr static auto ClassName = "Favorites"_ct;

		FavoritesModel::FavoritesItem ToFavoritesItem () const
		{
			return
			{
				Title_,
				URL_,
				Tags_.split (" ", Qt::SkipEmptyParts)
			};
		}

		static Favorites FromFavoritesItem (const FavoritesModel::FavoritesItem& item)
		{
			return
			{
				item.Title_,
				item.URL_,
				item.Tags_.join (" ")
			};
		}
	};

	struct SQLStorageBackend::FormsNever
	{
		oral::PKey<QString, oral::NoAutogen> URL_;

		constexpr static auto ClassName = "Forms_Never"_ct;
	};
}
}

ORAL_ADAPT_STRUCT (LC::Poshuku::SQLStorageBackend::History,
		Date_,
		Title_,
		URL_)
ORAL_ADAPT_STRUCT (LC::Poshuku::SQLStorageBackend::Favorites,
		Title_,
		URL_,
		Tags_)
ORAL_ADAPT_STRUCT (LC::Poshuku::SQLStorageBackend::FormsNever,
		URL_)

namespace LC
{
namespace Poshuku
{
	SQLStorageBackend::SQLStorageBackend (StorageBackend::Type type)
	: Type_ { type }
	, DBGuard_ { Util::MakeScopeGuard ([this] { DB_.close (); }) }
	{
		QString strType;
		switch (type)
		{
		case SBSQLite:
			strType = "QSQLITE";
			break;
		case SBPostgres:
			strType = "QPSQL";
			break;
		}

		DB_ = QSqlDatabase::addDatabase (strType, Util::GenConnectionName ("org.LeechCraft.Poshuku"));
		switch (type)
		{
		case SBSQLite:
		{
			QDir dir = QDir::home ();
			dir.cd (".leechcraft");
			dir.cd ("poshuku");
			DB_.setDatabaseName (dir.filePath ("poshuku.db"));
			break;
		}
		case SBPostgres:
		{
			DB_.setDatabaseName (XmlSettingsManager::Instance ()->property ("PostgresDBName").toString ());
			DB_.setHostName (XmlSettingsManager::Instance ()->property ("PostgresHostname").toString ());
			DB_.setPort (XmlSettingsManager::Instance ()->property ("PostgresPort").toInt ());
			DB_.setUserName (XmlSettingsManager::Instance ()->property ("PostgresUsername").toString ());
			DB_.setPassword (XmlSettingsManager::Instance ()->property ("PostgresPassword").toString ());
			break;
		}
		}

		if (!DB_.open ())
		{
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error ("Could not initialize database");
		}

		if (type == SBSQLite)
			Util::RunTextQuery (DB_, "PRAGMA journal_model = WAL;");

		auto adaptedPtrs = std::tie (History_, Favorites_, FormsNever_);
		type == SBSQLite ?
				oral::AdaptPtrs<oral::SQLiteImplFactory> (DB_, adaptedPtrs) :
				oral::AdaptPtrs<oral::PostgreSQLImplFactory> (DB_, adaptedPtrs);
	}

	void SQLStorageBackend::LoadHistory (history_items_t& items) const
	{
		for (const auto& item : History_->Select.Build ().Order (oral::OrderBy<sph::desc<&History::Date_>>) ())
			items.push_back (item.ToHistoryItem ());
	}

	namespace
	{
		double Score (const QList<int>& diffs)
		{
			const auto k = 1 / 86400.; // decay rate should be the same order of magnitude as a day

			double result = 0;
			for (auto diff : diffs)
				result += std::exp (-k * diff);
			return result;
		}
	}

	history_items_t SQLStorageBackend::LoadResemblingHistory (const QString& base) const
	{
		using namespace oral::infix;

		const auto& pat = "%" + base + "%";
		const auto& allItems = History_->Select.Build ()
				.Select (sph::all)
				.Where (sph::f<&History::Title_> |like| pat || sph::f<&History::URL_> |like| pat)
				.Order (oral::OrderBy<sph::desc<&History::Date_>>)
				();

		const auto& now = QDateTime::currentDateTime ();

		QHash<QString, QString> url2title;
		QHash<QString, QList<int>> url2diffs;
		for (const auto& item : allItems)
		{
			if (item.URL_.startsWith ("data:"))
				continue;

			if (!url2title.contains (item.URL_))
				url2title [item.URL_] = item.Title_;

			url2diffs [item.URL_] << item.Date_->secsTo (now);
		}

		auto scored = Util::Map (Util::Stlize (url2diffs),
				[] (const auto& pair) { return QPair { pair.first, Score (pair.second) }; });
		std::sort (scored.rbegin (), scored.rend (), Util::ComparingBy (Util::Snd));

		return Util::Map (scored,
				[&url2title] (const auto& pair)
				{
					const auto& url = pair.first;
					return HistoryItem { url2title [url], {}, url };
				});
	}

	void SQLStorageBackend::AddToHistory (const HistoryItem& item)
	{
		const auto& record = History::FromHistoryItem (item);
		Type_ == SBSQLite ?
				History_->Insert (oral::SQLiteImplFactory {}, record, oral::InsertAction::Replace::PKey) :
				History_->Insert (oral::PostgreSQLImplFactory {}, record, oral::InsertAction::Replace::PKey);
		emit added (item);
	}

	void SQLStorageBackend::ClearOldHistory (int age, int items)
	{
		const auto& countDateThreshold = History_->SelectOne
				.Build ()
				.Select (sph::fields<&History::Date_>)
				.Order (oral::OrderBy<sph::asc<&History::Date_>>)
				.Limit (1)
				.Offset (items)
				();

		const auto& ageDateThreshold = QDateTime::currentDateTime ().addDays (-age);

		const auto& threshold = countDateThreshold ?
				std::min (*countDateThreshold, ageDateThreshold) :
				ageDateThreshold;
		History_->DeleteBy (sph::f<&History::Date_> < threshold);
	}

	void SQLStorageBackend::LoadFavorites (FavoritesModel::items_t& items) const
	{
		for (const auto& fav : Favorites_->Select.Build ().Order (oral::OrderBy<sph::desc<&Favorites::Title_>>) ())
			items.push_back (fav.ToFavoritesItem ());
	}

	void SQLStorageBackend::AddToFavorites (const FavoritesModel::FavoritesItem& item)
	{
		Favorites_->Insert (Favorites::FromFavoritesItem (item));
		emit added (item);
	}

	void SQLStorageBackend::RemoveFromFavorites (const FavoritesModel::FavoritesItem& item)
	{
		Favorites_->DeleteBy (sph::f<&Favorites::URL_> == item.URL_);
		emit removed (item);
	}

	void SQLStorageBackend::UpdateFavorites (const FavoritesModel::FavoritesItem& item)
	{
		Favorites_->Update (Favorites::FromFavoritesItem (item));
		emit updated (item);
	}

	void SQLStorageBackend::SetFormsIgnored (const QString& url, bool ignore)
	{
		if (ignore)
			FormsNever_->Insert ({ url });
		else
			FormsNever_->Delete ({ url });
	}

	bool SQLStorageBackend::GetFormsIgnored (const QString& url) const
	{
		return FormsNever_->Select (sph::count<>, sph::f<&FormsNever::URL_> == url);
	}
}
}
