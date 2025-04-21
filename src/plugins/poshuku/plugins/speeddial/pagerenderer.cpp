/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pagerenderer.h"
#include <functional>
#include <QSize>
#include <QUrlQuery>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/sll/xmlnode.h>
#include "imagecache.h"
#include "requesthandler.h"

namespace LC::Poshuku::SpeedDial
{
	namespace
	{
		QString GetPageHeader ()
		{
			return QObject::tr ("Speed dial");
		}

		QString MakeThumbUrl (const QUrl& siteUrl)
		{
			QUrl url { ThumbUrlBase };
			url.setQuery (QUrlQuery { { { ThumbUrlKey, siteUrl.toEncoded () } } });
			return url.toString ();
		}

		using Tag = Util::Tag;
		using Node = Util::Node;
		using Nodes = Util::Nodes;
		namespace Tags = Util::Tags;

		Tag MakeHead (ImageCache& cache)
		{
			const auto style = R"(
					.centered {
						margin-left: auto;
						margin-right: auto;
					}

					.thumbimage {
						display: block;
						border: 1px solid black;
					}

					p {
						white-space: nowrap;
						overflow: hidden;
						text-overflow: ellipsis;
						margin: 20px;
						text-align: center;
					}

					td > a {
						text-decoration: none;
						color: "#222";
					}

					table {
						margin-top: 10px;
					}

					th {
						text-align: center;
						font-size: 1.5em;
					}

					td {
						max-width: %1;
						min-width: %1;
						width: %1;
					}
				)"_qs;
			const auto& thumbSize = cache.GetThumbSize ();
			const auto& tdWidthStr = QString::number (thumbSize.width () + 20) + "px";
			return
			{
				"head"_qba, {},
				{
					Tags::Charset ("UTF-8"_qs),
					Tags::Title (QObject::tr ("Speed dial")),
					Tags::Style (style.arg (tdWidthStr)),
				}
			};
		}

		Nodes MakeCell (ImageCache& cache, const TopList_t& items, size_t row, size_t col)
		{
			const auto idx = static_cast<int> (row * Cols + col);
			if (idx >= items.size ())
				return {};

			const auto& [url, name] = items.at (idx);
			return
			{
				Tag
				{
					.Name_ = "a"_qba,
					.Attrs_ = { { "href"_qba, url.toEncoded () } },
					.Children_
					{
						Tags::Image (MakeThumbUrl (url), cache.GetThumbSize ())
								.WithAttr ("class"_qba, "thumbimage centered"_qs),
						Tags::P ({ name }),
					},
				}
			};
		}

		Node MakeTable (ImageCache& cache, const QPair<QString, TopList_t>& tableInfo)
		{
			const auto& heading = tableInfo.first;
			const auto& items = tableInfo.second;
			const auto& cellMaker = std::bind_front (&MakeCell, std::ref (cache), std::ref (items));
			return Tag
			{
				.Name_ = "table"_qba,
				.Attrs_ = { { "class"_qba, "centered"_qs } },
				.Children_ =
					Tag { "th"_qba, { { "colspan"_qba, QString::number (Cols) } }, { heading } } +
					Tags::TableGrid (Rows, Cols, cellMaker)
			};
		}
	}

	QByteArray MakePage (ImageCache& cache, const QList<QPair<QString, TopList_t>>& tables)
	{
		return Tags::Html ({
				MakeHead (cache),
				Tags::Body (Util::MapAs<QVector> (tables, std::bind_front (&MakeTable, std::ref (cache)))),
			}).Serialize (QByteArray { "<!DOCTYPE html>" });
	}

	QByteArray MakeEmptyTopList ()
	{
		const auto& msg = QObject::tr ("A static site list is selected for speed dial, but no sites are configured.");
		return "<html><head><title>%1</title></head><body>%2</body></html>"_qs
				.arg (GetPageHeader (), msg)
				.toUtf8 ();
	}
}
