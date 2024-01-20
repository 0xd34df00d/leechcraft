/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "epubloader.h"
#include <QDomDocument>
#include <QImageReader>
#include <QTextDocument>
#include <QtConcurrent>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <util/sll/domchildrenrange.h>
#include <util/sll/timer.h>
#include <util/sll/visitor.h>
#include <util/sll/qtutil.h>
#include "document.h"
#include "internallinks.h"
#include "manifest.h"
#include "microcsshandler.h"
#include "microcssparser.h"
#include "toc.h"
#include "util.h"

namespace LC::Monocle::Boop
{
	namespace
	{
		using Stylesheet = MicroCSS::Stylesheet;

		QVector<QDomElement> ExtractChapterBody (const QDomElement& root)
		{
			const auto& body = GetElem (root, "body"_qs);
			const auto& childrenRange = Util::DomChildren (body, {});
			return { childrenRange.begin (), childrenRange.end () };
		}

		struct LoadedChapters
		{
			QVector<QDomElement> AllChildren_;
			QSet<QString> ExternalStylesheets_;
			Stylesheet InternalStylesheet_;
		};

		QSet<QString> GetExternalStylesheets (const QDomElement& root)
		{
			static const auto cssMime = "text/css"_qs;

			QSet<QString> result;
			for (const auto& link : Util::DomDescendants (root, "link"_qs))
				if (link.attribute ("rel"_qs) == "stylesheet"_ql ||
					link.attribute ("type"_qs, cssMime) == cssMime)
					result << link.attribute ("href"_qs);
			return result;
		}

		bool IsCssSelectorRelevant (const MicroCSS::Selector& selector)
		{
			return Util::Visit (selector,
					[] (const MicroCSS::AtSelector&) { return false; },
					[] (const MicroCSS::ComplexSelector&) { return false; },
					[] (const auto&) { return true; });
		}

		Stylesheet GetInternalStylesheet (const QDomElement& root)
		{
			Stylesheet result;
			for (const auto& style : Util::DomDescendants (root, "style"_qs))
				result += MicroCSS::Parse (style.text (), &IsCssSelectorRelevant);
			return result;
		}

		LoadedChapters ExtractChapter (const QString& epubFile, const QString& subpath)
		{
			const auto& doc = GetXml (epubFile, subpath);
			const auto& root = doc.documentElement ();

			FixLinks (root, subpath);

			return { ExtractChapterBody (root), GetExternalStylesheets (root), GetInternalStylesheet (root) };
		}

		LoadedChapters CollectChapters (const QString& epubFile, const Manifest& manifest)
		{
			LoadedChapters chapters;
			for (const auto& partId : manifest.Spine_)
			{
				const auto& item = manifest.Id2Item_.value (partId);
				if (item.Path_.isEmpty ())
					throw InvalidEpub { "unknown contents " + partId };
				auto loadedChapter = ExtractChapter (epubFile, item.Path_);
				chapters.AllChildren_ += loadedChapter.AllChildren_;
				chapters.ExternalStylesheets_.unite (loadedChapter.ExternalStylesheets_);
				chapters.InternalStylesheet_ += loadedChapter.InternalStylesheet_;
			}
			return chapters;
		}

		Stylesheet LoadStylesheets (const QString& epubFile, const QSet<QString>& stylesheets)
		{
			Stylesheet result;
			for (const auto& ssFile : stylesheets)
			{
				QuaZipFile file { epubFile, ssFile, QuaZip::csInsensitive };
				if (!file.open (QIODevice::ReadOnly))
				{
					qWarning () << "unable to open" << ssFile << ":" << file.errorString ();
					continue;
				}

				result += MicroCSS::Parse (QString::fromUtf8 (file.readAll ()), &IsCssSelectorRelevant);
			}
			return result;
		}

		std::pair<QDomElement, Stylesheet> LoadSpine (const QString& epubFile, const Manifest& manifest)
		{
			Util::Timer timer;
			const auto& chapters = CollectChapters (epubFile, manifest);
			timer.Stamp ("extracting children");

			auto stylesheet = LoadStylesheets (epubFile, chapters.ExternalStylesheets_);
			stylesheet += chapters.InternalStylesheet_;
			timer.Stamp ("loading stylesheets");

			QDomDocument doc;
			auto body = doc.createElement ("body"_qs);
			for (const auto& elem : chapters.AllChildren_)
				body.appendChild (doc.importNode (elem, true));
			doc.appendChild (body);
			timer.Stamp ("uniting children");
			return { body, stylesheet };
		}

		LazyImages_t LoadImages (const QString& epubFile, const Manifest& manifest)
		{
			Util::Timer timer;
			QVector<PathItem> imageItems;
			imageItems.reserve (manifest.Id2Item_.size ());
			std::copy_if (manifest.Id2Item_.begin (), manifest.Id2Item_.end (), std::back_inserter (imageItems),
					[] (const PathItem& item) { return item.Mime_.startsWith ("image/"_ql); });

			using MaybeImagesList_t = QVector<std::optional<std::pair<QString, LazyImage>>>;
			const auto& images = QtConcurrent::blockingMapped<MaybeImagesList_t> (imageItems,
					std::function
					{
						[&] (const PathItem& item) -> std::optional<std::pair<QString, LazyImage>>
						{
							QuaZipFile file { epubFile, item.Path_, QuaZip::csInsensitive };
							if (!file.open (QIODevice::ReadOnly))
								throw InvalidEpub { "unable to open " + item.Path_ + ": " + file.errorString () };

							QImageReader reader { &file };
							const auto& nativeSize = reader.size ();
							if (nativeSize.isNull ())
							{
								qWarning () << "null image from" << item.Path_;
								return {};
							}

							const auto& image = reader.read ();

							return std::pair
							{
								item.Path_,
								LazyImage
								{
									nativeSize,
									[image] (QSize size) { return image.scaled (size, Qt::KeepAspectRatio, Qt::SmoothTransformation); }
								}
							};
						}
					});
			timer.Stamp ("parsing images");

			LazyImages_t result;
			result.reserve (images.size ());
			for (auto& maybeImage : images)
				if (maybeImage)
					result [maybeImage->first] = std::move (maybeImage->second);
			return result;
		}
	}

	IDocument_ptr LoadZip (const QString& epubFile, QObject *pluginObj)
	{
		try
		{
			const auto& manifest = ParseManifest (epubFile);
			const auto& toc = LoadTocMap (epubFile, manifest);
			auto [body, stylesheet] = LoadSpine (epubFile, manifest);
			MarkTocTargets (body, toc);

			const auto& images = LoadImages (epubFile, manifest);

			const auto& doc = std::make_shared<Document> (QUrl::fromLocalFile (epubFile), pluginObj);
			doc->SetDocument ({
					.BodyElem_ = body,
					.Images_ = images,
					.Styler_ = MicroCSS::MakeStyler (stylesheet)
				});
			return doc;
		}
		catch (const InvalidEpub& error)
		{
			qWarning () << "invalid EPUB file" << epubFile << ":" << error.Error_;
			return {};
		}
	}
}
