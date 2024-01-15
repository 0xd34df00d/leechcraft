/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "epubloader.h"
#include <QDomDocument>
#include <QTextDocument>
#include <QtConcurrent>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <util/sll/domchildrenrange.h>
#include <util/sll/timer.h>
#include <util/sll/visitor.h>
#include <util/sll/qtutil.h>
#include "document.h"
#include "microcsshandler.h"
#include "microcssparser.h"
#include "util.h"

namespace LC::Monocle::Boop
{
	namespace
	{
		QString FindOpfFile (const QString& epubFile)
		{
			const auto& doc = GetXml (epubFile, "META-INF/container.xml"_qs);
			const auto& rootfiles = GetElem (doc.documentElement (), "rootfiles"_qs);
			const auto& rootfile = GetElem (rootfiles, "rootfile"_qs);
			return GetAttr (rootfile, "full-path"_qs);
		}

		struct PathItem
		{
			QString Path_;
			QString Mime_;
		};

		struct Manifest
		{
			QHash<QString, PathItem> Id2Item_;
			QVector<QString> Spine_;
		};

		Manifest ParseManifest (const QString& epubFile, const QString& opfFile)
		{
			const auto& doc = GetXml (epubFile, opfFile);

			Manifest manifest;
			for (const auto& item : Util::DomChildren (GetElem (doc.documentElement (), "manifest"_qs), "item"_qs))
			{
				const auto& id = GetAttr (item, "id"_qs);
				const auto& href = GetAttr (item, "href"_qs);
				const auto& resolvedPath = QUrl { opfFile }.resolved (QUrl { href }).toString ();
				const auto& mime = GetAttr (item, "media-type"_qs);
				manifest.Id2Item_ [id] = PathItem { resolvedPath, mime };
			}

			const auto& spine = GetElem (doc.documentElement(), "spine"_qs);
			for (const auto& item : Util::DomChildren (spine, "itemref"_qs))
				manifest.Spine_ << GetAttr (item, "idref"_qs);

			return manifest;
		}

		void ResolveLinks (const QString& tagName, const QString& linkAttr, const QDomElement& root, const QUrl& baseUrl)
		{
			for (auto image : Util::DomDescendants (root, tagName))
				if (const auto& link = image.attribute (linkAttr);
					!link.isEmpty ())
				{
					const auto linkUrl = QUrl::fromEncoded (link.toLatin1 ());
					if (linkUrl.isRelative ())
						image.setAttribute (linkAttr, baseUrl.resolved (linkUrl).toString ());
				}
		}

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

		QString FixupAnchor (QString anchor)
		{
			return anchor.replace ('#', '_').replace ('/', '_');
		}

		void FixupLinkHrefAnchors (const QDomElement &root)
		{
			for (auto link : Util::DomDescendants (root, "a"_qs))
			{
				auto href = link.attribute ("href"_qs);
				if (href.isEmpty () || !QUrl::fromEncoded (href.toUtf8 ()).isRelative ())
					continue;

				link.setAttribute ("href"_qs, FixupAnchor (std::move (href)).prepend ('#'));
			}
		}

		void FixupIdAnchors (QDomElement elem, const QString& subpath)
		{
			if (const auto& id = elem.attribute ("id"_qs);
				!id.isEmpty ())
				elem.setAttribute ("id"_qs, FixupAnchor (subpath + '#' + id));

			for (const auto& child : Util::DomChildren (elem, {}))
				FixupIdAnchors (child.toElement (), subpath);
		}

		void FixLinks (const QDomElement& root, const QString& subpath)
		{
			const QUrl chapterBaseUrl { subpath };
			ResolveLinks ("img"_qs, "src"_qs, root, chapterBaseUrl);
			ResolveLinks ("link"_qs, "href"_qs, root, chapterBaseUrl);

			ResolveLinks ("a"_qs, "href"_qs, root, chapterBaseUrl);
			FixupLinkHrefAnchors (root);
			FixupIdAnchors (root, subpath);
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

		ImagesList_t LoadImages (const QString& epubFile, const Manifest& manifest)
		{
			Util::Timer timer;
			QVector<PathItem> imageItems;
			imageItems.reserve (manifest.Id2Item_.size ());
			std::copy_if (manifest.Id2Item_.begin (), manifest.Id2Item_.end (), std::back_inserter (imageItems),
					[] (const PathItem& item) { return item.Mime_.startsWith ("image/"_ql); });

			using LocatedImage_t = LocatedImage_t;
			using MaybeImagesList_t = QVector<std::optional<LocatedImage_t>>;
			const auto& images = QtConcurrent::blockingMapped<MaybeImagesList_t> (imageItems,
					std::function
					{
						[&] (const PathItem& item) -> std::optional<LocatedImage_t>
						{
							QuaZipFile file { epubFile, item.Path_, QuaZip::csInsensitive };
							if (!file.open (QIODevice::ReadOnly))
								throw InvalidEpub { "unable to open " + item.Path_ + ": " + file.errorString () };

							auto image = QImage::fromData (file.readAll ());
							if (image.isNull ())
							{
								qWarning () << "null image from" << item.Path_;
								return {};
							}

							return LocatedImage_t { item.Path_, image };
						}
					});
			timer.Stamp ("parsing images");

			ImagesList_t result;
			result.reserve (images.size ());
			for (const auto& maybeImage : images)
				if (maybeImage)
					result << *maybeImage;
			return result;
		}
	}

	IDocument_ptr LoadZip (const QString& epubFile, QObject *pluginObj)
	{
		try
		{
			const auto& opfFile = FindOpfFile (epubFile);
			const auto& manifest = ParseManifest (epubFile, opfFile);
			const auto& [body, stylesheet] = LoadSpine (epubFile, manifest);

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
