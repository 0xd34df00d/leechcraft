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
#include <QtConcurrentMap>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <util/sll/domchildrenrange.h>
#include <util/sll/timer.h>
#include <util/sll/prelude.h>
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

		auto GetXml (const QString& epubFile, const QString& filename)
		{
			QuaZipFile file { epubFile, filename, QuaZip::csInsensitive };
			if (!file.open (QIODevice::ReadOnly))
				throw InvalidEpub { "unable to open " + filename + ": " + file.errorString () };

			QDomDocument doc;
			QString errorMsg;
			if (!doc.setContent (&file, false, &errorMsg))
				throw InvalidEpub { "unable to parse " + filename + ": " + errorMsg };
			return doc;
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

		struct Manifest
		{
			QHash<QString, QString> Id2Path_;
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
				manifest.Id2Path_ [id] = QUrl { opfFile }.resolved (QUrl { href }).toString ();
			}

			const auto& spine = GetElem (doc.documentElement(), "spine"_qs);
			for (const auto& item : Util::DomChildren (spine, "itemref"_qs))
				manifest.Spine_ << GetAttr (item, "idref"_qs);

			return manifest;
		}

		QVector<QDomElement> ExtractBodyChildren (const QString& epubFile, const QString& subpath)
		{
			const auto& doc = GetXml (epubFile, subpath);
			const auto& body = GetElem (doc.documentElement (), "body"_qs);

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
			QVector<QString> paths;
			for (const auto& partId : manifest.Spine_)
			{
				const auto& path = manifest.Id2Path_.value (partId);
				if (path.isEmpty ())
					throw InvalidEpub { "unknown ref " + path };
				paths << path;
			}

			using Elems_t = QVector<QDomElement>;

			constexpr auto parallelThreshold = 50;
			if (paths.size () > parallelThreshold)
			{
				auto subresults = QtConcurrent::blockingMapped<QVector<Elems_t>> (paths,
						std::function { [&] (const QString& path) { return ExtractBodyChildren (epubFile, path); } });
				return Util::Concat (std::move (subresults));
			}

			Elems_t bodiesChildren;
			for (const auto& path : paths)
				bodiesChildren += ExtractBodyChildren (epubFile, path);
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
	}

	IDocument_ptr LoadZip (const QString& epubFile, QObject *pluginObj)
	{
		try
		{
			const auto& opfFile = FindOpfFile (epubFile);
			const auto& manifest = ParseManifest (epubFile, opfFile);
			const auto& contents = LoadSpine (epubFile, manifest);

			Util::Timer timer;
			auto textDoc = std::make_unique<QTextDocument> ();
			textDoc->setHtml (contents);
			TextDocumentFormatConfig::Instance ().FormatDocument (*textDoc);
			timer.Stamp ("creating doc");

			return std::make_shared<Document> (std::move (textDoc), QUrl::fromLocalFile (epubFile), pluginObj);
		}
		catch (const InvalidEpub& error)
		{
			qWarning () << "invalid EPUB file" << epubFile << ":" << error.Error_;
			return {};
		}
	}
}
