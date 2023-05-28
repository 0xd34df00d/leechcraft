/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "icondatabaseondisk.h"
#include <QUrl>
#include <QIcon>
#include <QDir>
#include <QSqlError>
#include <util/db/oral/oral.h>
#include <util/db/oral/utilitytypes.h>
#include <util/sys/paths.h>

namespace LC::Poshuku::WebEngineView
{
	struct IconDatabaseOnDisk::IconUrl2IconRecord
	{
		Util::oral::PKey<QUrl, Util::oral::NoAutogen> IconUrl_;
		Util::oral::AsDataStream<QIcon> Icon_;

		constexpr static auto ClassName = "IconUrl2Icon"_ct;
	};

	struct IconDatabaseOnDisk::PageUrl2IconUrlRecord
	{
		Util::oral::PKey<QUrl, Util::oral::NoAutogen> PageUrl_;
		Util::oral::References<&IconUrl2IconRecord::IconUrl_> IconUrl_;
		QDateTime LastUpdate_;

		constexpr static auto ClassName = "PageUrl2IconUrl"_ct;
	};
}

using IDOD = LC::Poshuku::WebEngineView::IconDatabaseOnDisk;

BOOST_FUSION_ADAPT_STRUCT (IDOD::IconUrl2IconRecord,
		IconUrl_,
		Icon_)

BOOST_FUSION_ADAPT_STRUCT (IDOD::PageUrl2IconUrlRecord,
		PageUrl_,
		IconUrl_,
		LastUpdate_)

namespace LC::Poshuku::WebEngineView
{
	IconDatabaseOnDisk::IconDatabaseOnDisk ()
	: DB_ { QSqlDatabase::addDatabase ("QSQLITE",
			Util::GenConnectionName ("org.LeechCraft.Poshuku.WebEngineView.IconDB")) }
	{
		const auto& cacheDir = Util::GetUserDir (Util::UserDir::Cache, "poshuku/webengineview");
		DB_.setDatabaseName (cacheDir.filePath ("icons.db"));

		if (!DB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open the database";
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error { "Cannot create database" };
		}

		Util::RunTextQuery (DB_, "PRAGMA synchronous = NORMAL;");
		Util::RunTextQuery (DB_, "PRAGMA journal_mode = WAL;");

		IconUrl2Icon_ = Util::oral::AdaptPtr<IconUrl2IconRecord> (DB_);
		PageUrl2IconUrl_ = Util::oral::AdaptPtr<PageUrl2IconUrlRecord> (DB_);
	}

	namespace sph = Util::oral::sph;

	void IconDatabaseOnDisk::UpdateIcon (const QUrl& pageUrl, const QIcon& icon, const QUrl& iconUrl)
	{
		using Replace = Util::oral::InsertAction::Replace;

		const auto& now = QDateTime::currentDateTime ();

		IconUrl2Icon_->Insert ({ iconUrl, icon }, Replace::PKey);
		PageUrl2IconUrl_->Insert ({ pageUrl, iconUrl, now }, Replace::PKey);
	}

	QIcon IconDatabaseOnDisk::GetIcon (const QUrl& iconUrl)
	{
		return IconUrl2Icon_->SelectOne (sph::fields<&IconUrl2IconRecord::Icon_>,
				sph::f<&IconUrl2IconRecord::IconUrl_> == iconUrl)
				.value_or (Util::oral::AsDataStream<QIcon> {});
	}

	QList<std::tuple<QUrl, QUrl>> IconDatabaseOnDisk::GetAllPages () const
	{
		return PageUrl2IconUrl_->Select (sph::fields<&PageUrl2IconUrlRecord::PageUrl_, &PageUrl2IconUrlRecord::IconUrl_>);
	}
}
