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
#include <QThread>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <util/sll/domchildrenrange.h>
#include <util/sll/timer.h>
#include <util/sll/qtutil.h>
#include <util/monocle/textdocumentformatconfig.h>
#include "document.h"

namespace LC::Monocle::Boop
{
	namespace
	{
		struct InvalidEpub final : std::exception
		{
			QString Error_;

			InvalidEpub (QString error)
			: Error_ { std::move (error) }
			{
			}

			~InvalidEpub () override = default;

			InvalidEpub (const InvalidEpub&) = delete;
			InvalidEpub (InvalidEpub&&) = delete;
			InvalidEpub& operator= (const InvalidEpub&) = delete;
			InvalidEpub& operator= (InvalidEpub&&) = delete;
		};

		auto ParseXml (auto&& xmlable, const auto& context)
		{
			QDomDocument doc;
			QString errorMsg;
			if (!doc.setContent (xmlable, false, &errorMsg))
				throw InvalidEpub { "unable to parse xml " + context };
			return doc;
		}

		auto GetXml (const QString& epubFile, const QString& filename)
		{
			QuaZipFile file { epubFile, filename, QuaZip::csInsensitive };
			if (!file.open (QIODevice::ReadOnly))
				throw InvalidEpub { "unable to open " + filename + ": " + file.errorString () };

			return ParseXml (&file, filename);
		}

		auto GetElem (const QDomElement& parent, const QString& tag)
		{
			const auto& result = parent.firstChildElement (tag);
			if (result.isNull ())
				throw InvalidEpub { tag + " is empty" };
			return result;
		}

		auto GetAttr (const QDomElement& elem, const QString& name)
		{
			const auto& attrValue = elem.attribute (name);
			if (attrValue.isEmpty ())
				throw InvalidEpub { name + " is empty" };
			return attrValue;
		}

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

		void FixupLinks (const QDomElement& root, const QString& basePath)
		{
			auto images = root.elementsByTagName ("img"_qs);
			const QUrl baseUrl { basePath };
			for (int i = 0; i < images.size (); ++i)
			{
				auto image = images.at (i).toElement ();
				const auto& src = image.attribute ("src"_qs);
				if (src.isEmpty ())
					continue;
				image.setAttribute ("src"_qs, baseUrl.resolved (QUrl { src }).toString ());
			}
		}

		QVector<QDomElement> ExtractBodyChildren (const QString& epubFile, const QString& subpath)
		{
			const auto& doc = GetXml (epubFile, subpath);
			const auto& body = GetElem (doc.documentElement (), "body"_qs);

			FixupLinks (body, subpath);

			const auto& bodyChildren = body.childNodes ();
			const auto childrenCount = bodyChildren.size ();

			QVector<QDomElement> result;
			result.reserve (childrenCount);
			for (int i = 0; i < childrenCount; ++i)
			{
				const auto& node = bodyChildren.at (i);
				if (node.isElement ())
					result << node.toElement ();
			}

			return result;
		}

		QVector<QDomElement> CollectChildren (const QString& epubFile, const Manifest& manifest)
		{
			QVector<QDomElement> bodiesChildren;
			for (const auto& partId : manifest.Spine_)
			{
				const auto& item = manifest.Id2Item_.value (partId);
				if (item.Path_.isEmpty ())
					throw InvalidEpub { "unknown contents " + partId };
				bodiesChildren += ExtractBodyChildren (epubFile, item.Path_);
			}
			return bodiesChildren;
		}

		QString LoadSpine (const QString& epubFile, const Manifest& manifest)
		{
			Util::Timer timer;
			const auto& bodiesChildren = CollectChildren (epubFile, manifest);
			timer.Stamp ("extracting children");

			QDomDocument doc;
			auto body = doc.createElement ("body"_qs);
			for (const auto& elem : bodiesChildren)
				body.appendChild (doc.importNode (elem, true));
			doc.appendChild (body);
			timer.Stamp ("uniting children");
			return doc.toString ();
		}

		void LoadImages (const QString& epubFile, const Manifest& manifest, QTextDocument& textDoc)
		{
			for (const auto& item : manifest.Id2Item_)
			{
				if (!item.Mime_.startsWith ("image/"_ql))
					continue;

				QuaZipFile file { epubFile, item.Path_, QuaZip::csInsensitive };
				if (!file.open (QIODevice::ReadOnly))
					throw InvalidEpub { "unable to open " + item.Path_ + ": " + file.errorString () };

				auto image = QImage::fromData (file.readAll ());
				if (image.isNull ())
				{
					qWarning () << "null image from" << item.Path_;
					continue;
				}

				textDoc.addResource (QTextDocument::ImageResource, QUrl { item.Path_ }, QVariant::fromValue (image));
			}
		}
	}

	IDocument_ptr LoadZip (const QString& epubFile, QObject *pluginObj)
	{
		try
		{
			auto textDoc = std::make_unique<QTextDocument> ();

			const auto& opfFile = FindOpfFile (epubFile);
			const auto& manifest = ParseManifest (epubFile, opfFile);
			const auto& contents = LoadSpine (epubFile, manifest);

			Util::Timer timer;
			textDoc->setHtml (contents);
			TextDocumentFormatConfig::Instance ().FormatDocument (*textDoc);
			timer.Stamp ("creating doc");
			LoadImages (epubFile, manifest, *textDoc);
			timer.Stamp ("loading images");

			return std::make_shared<Document> (std::move (textDoc), QUrl::fromLocalFile (epubFile), pluginObj);
		}
		catch (const InvalidEpub& error)
		{
			qWarning () << "invalid EPUB file" << epubFile << ":" << error.Error_;
			return {};
		}
	}
}
