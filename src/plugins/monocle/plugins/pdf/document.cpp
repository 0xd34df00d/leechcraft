/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "document.h"
#include <thread>
#include <QThread>
#include <QtDebug>
#include <QBuffer>
#include <QFile>
#include <QtConcurrentRun>
#include "qt5compat.h"
#include <poppler-qt5.h>
#include <poppler-form.h>
#include <poppler-version.h>
#include <util/sll/util.h>
#include <util/sll/domchildrenrange.h>
#include <util/threads/futures.h>
#include "links.h"
#include "fields.h"
#include "annotations.h"
#include "xmlsettingsmanager.h"
#include "pendingfontinforequest.h"

namespace LC
{
namespace Monocle
{
namespace PDF
{
	Document::Document (const QString& path, QObject *plugin)
	: PDocument_ (Poppler::Document::load (path))
	, DocURL_ (QUrl::fromLocalFile (path))
	, Plugin_ (plugin)
	{
		if (!PDocument_)
			return;

		auto setRenderHint = [this] (const QByteArray& optName, Poppler::Document::RenderHint hint)
		{
			PDocument_->setRenderHint (hint,
					XmlSettingsManager::Instance ().property (optName).toBool ());
		};
		setRenderHint ("EnableAntialiasing", Poppler::Document::Antialiasing);
		setRenderHint ("EnableTextAntialiasing", Poppler::Document::TextAntialiasing);
		setRenderHint ("EnableTextHinting", Poppler::Document::TextHinting);
		setRenderHint ("EnableTextSlightHinting", Poppler::Document::TextSlightHinting);

		const auto& enhanceMode = XmlSettingsManager::Instance ()
				.property ("ThinLineEnhancement").toString ();
		if (enhanceMode == "Solid")
			PDocument_->setRenderHint (Poppler::Document::ThinLineSolid);
		else if (enhanceMode == "Shape")
			PDocument_->setRenderHint (Poppler::Document::ThinLineShape);

		BuildTOC ();
	}

	QObject* Document::GetBackendPlugin () const
	{
		return Plugin_;
	}

	QObject* Document::GetQObject ()
	{
		return this;
	}

	bool Document::IsValid () const
	{
		return PDocument_ && !PDocument_->isLocked ();
	}

	DocumentInfo Document::GetDocumentInfo () const
	{
		DocumentInfo info;
		if (!PDocument_)
			return info;

		info.Title_ = PDocument_->info ("Title");
		info.Subject_ = PDocument_->info ("Subject");
		info.Author_ = PDocument_->info ("Author");
		return info;
	}

	int Document::GetNumPages () const
	{
		return PDocument_->numPages ();
	}

	QSize Document::GetPageSize (int num) const
	{
		std::unique_ptr<Poppler::Page> page (PDocument_->page (num));
		if (!page)
			return QSize ();

		return page->pageSize ();
	}

	QFuture<QImage> Document::RenderPage (int num, double xScale, double yScale)
	{
		std::shared_ptr<Poppler::Page> page (PDocument_->page (num));
		if (!page)
			return Util::MakeReadyFuture (QImage {});

		return QtConcurrent::run ([=] { return page->renderToImage (72 * xScale, 72 * yScale); });
	}

	QList<ILink_ptr> Document::GetPageLinks (int num)
	{
		QList<ILink_ptr> result;
		std::unique_ptr<Poppler::Page> page (PDocument_->page (num));
		if (!page)
			return result;

		for (const auto link : page->links ())
			result << std::make_shared<Link> (this, link);

		return result;
	}

	QUrl Document::GetDocURL () const
	{
		return DocURL_;
	}

	TOCEntryLevel_t Document::GetTOC ()
	{
		return TOC_;
	}

	QString Document::GetTextContent (int pageNum, const QRect& rect)
	{
		std::unique_ptr<Poppler::Page> page (PDocument_->page (pageNum));
		if (!page)
			return QString ();

		return page->text (rect);
	}

	QAbstractItemModel* Document::GetOptContentModel ()
	{
		return PDocument_->hasOptionalContent () ?
				PDocument_->optionalContentModel () :
				nullptr;
	}

	IPendingFontInfoRequest* Document::RequestFontInfos () const
	{
		return new PendingFontInfoRequest (PDocument_);
	}

	QList<IAnnotation_ptr> Document::GetAnnotations (int pageNum)
	{
		std::unique_ptr<Poppler::Page> page (PDocument_->page (pageNum));
		if (!page)
			return {};

		QList<IAnnotation_ptr> annotations;
		for (const auto ann : page->annotations ())
			if (const auto wrapper = MakeAnnotation (this, ann))
				annotations << wrapper;
		return annotations;
	}

	IFormFields_t Document::GetFormFields (int pageNum)
	{
		std::unique_ptr<Poppler::Page> page (PDocument_->page (pageNum));
		if (!page)
			return IFormFields_t ();

		QList<std::shared_ptr<Poppler::FormField>> popplerFields;
		for (auto field : page->formFields ())
			popplerFields << std::shared_ptr<Poppler::FormField> (field);

		IFormFields_t fields;
		for (auto field : popplerFields)
		{
			if (!field->isVisible ())
				continue;

			switch (field->type ())
			{
			case Poppler::FormField::FormText:
				fields << std::make_shared<FormFieldText> (field);
				break;
			case Poppler::FormField::FormChoice:
				fields << std::make_shared<FormFieldChoice> (field);
				break;
			case Poppler::FormField::FormButton:
				fields << std::make_shared<FormFieldButton> (field, this);
				break;
			default:
				break;
			}
		}
		return fields;
	}

	void Document::PaintPage (QPainter *painter, int num, double xScale, double yScale)
	{
		const auto backend = PDocument_->renderBackend ();
		Util::DefaultScopeGuard guard;
		if (backend != Poppler::Document::ArthurBackend)
		{
			PDocument_->setRenderBackend (Poppler::Document::ArthurBackend);
			guard = Util::MakeScopeGuard ([this, backend]
					{ PDocument_->setRenderBackend (backend); });
		}

		std::unique_ptr<Poppler::Page> page (PDocument_->page (num));
		if (!page)
			return;

		page->renderToPainter (painter, 72 * xScale, 72 * yScale);
	}

	QMap<int, QList<QRectF>> Document::GetTextPositions (const QString& text, Qt::CaseSensitivity cs)
	{
		typedef QMap<int, QList<QRectF>> Result_t;
		Result_t result;
		Poppler::Page::SearchFlags searchFlags;
		if (cs != Qt::CaseSensitive)
			searchFlags |= Poppler::Page::SearchFlag::IgnoreCase;

		const auto numPages = PDocument_->numPages ();

		QVector<QList<QRectF>> resVec;
		resVec.resize (numPages);

		std::vector<std::thread> threads;

		auto worker = [&resVec, &searchFlags, &text, this] (int start, int count)
		{
			std::unique_ptr<Poppler::Document> doc (Poppler::Document::load (DocURL_.toLocalFile ()));
			for (auto i = start, end = start + count; i < end; ++i)
			{
				std::unique_ptr<Poppler::Page> p (doc->page (i));
				resVec [i] = p->search (text, searchFlags);

				const auto& size = p->pageSizeF ();
				auto scaleMat = QMatrix {}.scale (1 / size.width (), 1 / size.height ());
				for (auto& res : resVec [i])
					res = scaleMat.mapRect (res);
			}
		};
		const auto threadCount = QThread::idealThreadCount ();
		const auto packSize = numPages / threadCount;
		for (int i = 0; i < threadCount; ++i)
			threads.emplace_back (worker, i * packSize, (i == threadCount - 1) ? (numPages % threadCount) : packSize);

		for (auto& thread : threads)
			thread.join ();

		for (int i = 0; i < numPages; ++i)
			if (!resVec.at (i).isEmpty ())
				result [i] = resVec.at (i);

		return result;
	}

	auto Document::CanSave () const -> SaveQueryResult
	{
		if (PDocument_->isEncrypted ())
			return { false, tr ("saving encrypted documents is not supported") };

		return { true, QString () };
	}

	bool Document::Save (const QString& path)
	{
		std::unique_ptr<Poppler::PDFConverter> conv (PDocument_->pdfConverter ());
		conv->setPDFOptions (Poppler::PDFConverter::WithChanges);

		if (path == DocURL_.toLocalFile ())
		{
			QBuffer buffer;
			buffer.open (QIODevice::WriteOnly);
			conv->setOutputDevice (&buffer);
			if (!conv->convert ())
				return false;

			QFile file (path);
			if (!file.open (QIODevice::WriteOnly))
				return false;

			file.write (buffer.buffer ());
			return true;
		}
		else
		{
			conv->setOutputFileName (path);
			return conv->convert ();
		}
	}

	void Document::RequestNavigation (const QString& filename, int page, double x, double y)
	{
		emit navigateRequested (filename, { page, { x, y } });
	}

	void Document::RequestPrinting ()
	{
		emit printRequested ({});
	}

	namespace
	{
		template<typename T>
		TOCEntryLevel_t BuildTOCLevel (Document *doc, PDocument_ptr pDoc, const T& levelRoot)
		{
			TOCEntryLevel_t result;

			for (const auto& elem : Util::DomChildren (levelRoot, {}))
			{
				const auto& name = elem.tagName ();

				ILink_ptr link;
				const auto& destStr = elem.attribute ("Destination");
				if (!destStr.isEmpty ())
					link = std::make_shared<TOCLink> (doc, std::make_unique<Poppler::LinkDestination> (destStr));
				else
				{
					const auto& destName = elem.attribute ("DestinationName");
					if (destName.isEmpty ())
					{
						qWarning () << Q_FUNC_INFO
								<< "empty destination name, dunno how to handle that";
						continue;
					}

					std::unique_ptr<Poppler::LinkDestination> dest { pDoc->linkDestination (destName) };
					if (!dest)
					{
						qWarning () << Q_FUNC_INFO
								<< "empty destination for"
								<< destName;
						continue;
					}
					link = std::make_shared<TOCLink> (doc, std::move (dest));
				}

				TOCEntry entry =
				{
					link,
					name,
					BuildTOCLevel (doc, pDoc, elem)
				};
				result << entry;
			}

			return result;
		}
	}

	void Document::BuildTOC ()
	{
		std::unique_ptr<QDomDocument> doc (PDocument_->toc ());
		if (!doc)
			return;
		TOC_ = BuildTOCLevel (this, PDocument_, *doc);
	}
}
}
}
