/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <QFont>
#include <QMargins>
#include <QString>
#include <QStringList>
#include <QPageSize>

namespace LC::Aggregator
{
	struct OPMLItem
	{
		QString URL_;
		QString HTMLUrl_;
		QString Title_;
		QString Description_;
		QStringList Categories_;
		int MaxArticleAge_;
		int FetchInterval_;
		int MaxArticleNumber_;
		bool CustomFetchInterval_;
	};

	struct ExportFileError
	{
		QString Error_;
	};

	struct ExportConfig
	{
		QString Title_;
		QString OwnerName_;
		QString OwnerEmail_;
	};

	struct Fb2Config
	{
		QString Title_;
	};

	struct PdfConfig
	{
		QString Title_;

		QFont Font_;
		int FontSize_;
		QMargins Margins_;
		QPageSize PageSize_;

		QString Filename_;
	};

	struct ItemsExportFormat : std::variant<Fb2Config, PdfConfig>
	{
		using variant::variant;
	};
}
