/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "requesthandler.h"
#include <cmath>
#include <QUrlQuery>
#include <QXmlStreamWriter>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <interfaces/poshuku/istoragebackend.h>
#include <interfaces/poshuku/iproxyobject.h>
#include "customsitesmanager.h"
#include "imagecache.h"
#include "xmlsettingsmanager.h"

namespace LC::Poshuku::SpeedDial
{
	extern const QString SpeedDialHost = "speeddial"_qs;
	extern const QString SpeedDialUrl = "lc://speeddial"_qs;

	using HandleResult = IInternalSchemeHandler::HandleResult;
	using ReplyContents = IInternalSchemeHandler::ReplyContents;

	namespace
	{
		const size_t Rows = 2;
		const size_t Cols = 4;

		const QString ThumbPath = "/thumb"_qs;
		const QString ThumbUrlBase = SpeedDialUrl + ThumbPath;
		const QString ThumbUrlKey = "url"_qs;

		QString GetPageHeader ()
		{
			return QObject::tr ("Speed dial");
		}

		QByteArray MakeEmptyTopList ()
		{
			const auto& msg = QObject::tr ("A static site list is selected for speed dial, but no sites are configured.");
			return "<html><head><title>%1</title></head><body>%2</body></html>"_qs
					.arg (GetPageHeader (), msg)
					.toUtf8 ();
		}

		QString MakeThumbUrl (const QUrl& siteUrl)
		{
			QUrl url { ThumbUrlBase };
			url.setQuery (QUrlQuery { { { ThumbUrlKey, siteUrl.toEncoded () } } });
			return url.toString ();
		}

		void WriteTable (ImageCache& cache, QXmlStreamWriter& w,
				const TopList_t& items, size_t rows, size_t cols,
				const QString& heading)
		{
			const auto& thumbSize = cache.GetThumbSize ();

			w.writeStartElement ("table");
			w.writeAttribute ("class", "centered");
			w.writeAttribute ("style", "margin-top: 10px");

			w.writeStartElement ("th");
			w.writeAttribute ("style", "text-align: center; font-size: 1.5em;");
			w.writeAttribute ("colspan", QString::number (cols));
			w.writeCharacters (heading);
			w.writeEndElement ();

			const auto& tdWidthStr = QString::number (thumbSize.width () + 20) + "px";

			for (size_t r = 0; r < rows; ++r)
			{
				w.writeStartElement ("tr");
				for (size_t c = 0; c < cols; ++c)
				{
					if (r * cols + c >= static_cast<size_t> (items.size ()))
						continue;

					const auto& item = items.at (r * cols + c);

					w.writeStartElement ("td");
						w.writeAttribute ("style",
								QString { "max-width: %1; min-width: %1; width: %1;" }
										.arg (tdWidthStr));
						w.writeStartElement ("a");
							w.writeAttribute ("href", item.first.toEncoded ());
							w.writeAttribute ("class", "sdlink");

							w.writeStartElement ("img");
								w.writeAttribute ("src", MakeThumbUrl (item.first));
								w.writeAttribute ("width", QString::number (thumbSize.width ()));
								w.writeAttribute ("height", QString::number (thumbSize.height ()));
								w.writeAttribute ("class", "thumbimage centered");
							w.writeEndElement ();

							w.writeStartElement ("p");
								w.writeAttribute ("class", "thumbtext");
								w.writeCharacters (item.second);
							w.writeEndElement ();
						w.writeEndElement ();
					w.writeEndElement ();
				}
				w.writeEndElement ();
			}

			w.writeEndElement ();
		}


		QByteArray MakeTables (ImageCache& cache, const QList<QPair<QString, TopList_t>>& tables)
		{
			QByteArray html = "<!DOCTYPE html>";
			QXmlStreamWriter w (&html);
			w.writeStartElement ("html");
			w.writeAttribute ("xmlns", "http://www.w3.org/1999/xhtml");
				w.writeStartElement ("head");
					w.writeStartElement ("meta");
						w.writeAttribute ("charset", "UTF-8");
					w.writeEndElement ();
					w.writeTextElement ("title", QObject::tr ("Speed dial"));
					w.writeTextElement ("style", R"delim(
							.centered {
								margin-left: auto;
								margin-right: auto;
							}

							.thumbimage {
								display: block;
								border: 1px solid black;
							}

							.thumbtext {
								white-space: nowrap;
								overflow: hidden;
								text-overflow: ellipsis;
								margin: 20px;
								text-align: center;
							}

							.sdlink {
								text-decoration: none;
								color: "#222";
							}
						)delim");
				w.writeEndElement ();
				w.writeStartElement ("body");
					for (const auto& table : tables)
						WriteTable (cache, w, table.second, Rows, Cols, table.first);
				w.writeEndElement ();
			w.writeEndElement ();

			return html;
		}

		double GetScore (const QDateTime& then, const QDateTime& now)
		{
			return std::log (std::max<int> (then.daysTo (now) + 1, 1));
		}

		template<typename K, typename V>
		auto GetSortedVec (const QHash<K, V>& hash)
		{
			std::vector<std::pair<K, V>> vec { hash.keyValueBegin (), hash.keyValueEnd () };
			std::sort (vec.rbegin (), vec.rend (), Util::ComparingBy (Util::Snd));
			return vec;
		}

		struct LoadResult
		{
			TopList_t TopPages_;
			TopList_t TopHosts_;
		};

		LoadResult GetTopUrls (const IStorageBackend_ptr& sb, size_t count)
		{
			history_items_t items;
			sb->LoadHistory (items);

			const auto& now = QDateTime::currentDateTime ();

			QHash<QString, double> url2score;
			QHash<QStringView, double> host2score;
			for (const auto& item : items)
			{
				const auto score = GetScore (item.DateTime_, now);
				url2score [item.URL_] += score;

				const auto startPos = item.URL_.indexOf ("//") + 2;
				const auto endPos = item.URL_.indexOf ('/', startPos);
				if (startPos >= 0 && endPos > startPos)
					host2score [QStringView { item.URL_ }.left (endPos + 1)] += score;
			}
			const auto& hostsVec = GetSortedVec (host2score);

			TopList_t topSites;
			for (size_t i = 0; i < std::min (hostsVec.size (), count); ++i)
			{
				const auto& url = hostsVec [i].first.toString ();
				topSites.append ({ url, url });

				url2score.remove (url);
			}

			const auto& vec = GetSortedVec (url2score);

			TopList_t topPages;
			for (size_t i = 0; i < std::min (vec.size (), count); ++i)
			{
				const auto& url = vec [i].first;

				const auto& item = std::find_if (items.begin (), items.end (),
						[&url] (const HistoryItem& item) { return item.URL_ == url; });

				topPages.append ({ url, item->Title_ });
			}

			return { topPages, topSites };
		}

		QByteArray MakeRootPage (const RootPageDeps& deps)
		{
			if (XmlSettingsManager::Instance ().property ("UseStaticList").toBool ())
			{
				const auto& topList = deps.CustomSites_.GetTopList ();
				return topList.isEmpty () ?
						MakeEmptyTopList () :
						MakeTables (deps.ImageCache_, { { {}, topList } });
			}
			else
			{
				const auto sb = deps.PoshukuProxy_.CreateStorageBackend ();
				const auto& result = GetTopUrls (sb, Rows * Cols);
				return MakeTables (deps.ImageCache_,
					{
						{ QObject::tr ("Top pages"), result.TopPages_ },
						{ QObject::tr ("Top sites"), result.TopHosts_ }
					});
			}
		}
	}

	HandleResult HandleRequest (const QString& path, const QUrlQuery& query, const RootPageDeps& deps)
	{
		if (path.isEmpty ())
			return HandleResult::Right ({
					.ContentType_ = "text/html",
					.Contents_ = MakeRootPage (deps),
				});

		return HandleResult { IInternalSchemeHandler::Error::Unsupported };
	}
}
