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
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/sll/util.h>
#include <util/threads/futures.h>
#include "links.h"
#include "fields.h"
#include "annotations.h"
#include "xmlsettingsmanager.h"
#include "pendingfontinforequest.h"

namespace LC::Monocle::PDF
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
		if (enhanceMode == "Solid"_qs)
			PDocument_->setRenderHint (Poppler::Document::ThinLineSolid);
		else if (enhanceMode == "Shape"_qs)
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

		info.Title_ = PDocument_->info ("Title"_qs);
		info.Subject_ = PDocument_->info ("Subject"_qs);
		info.Author_ = PDocument_->info ("Author"_qs);
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
			return {};

		return page->pageSize ();
	}

	namespace
	{
		constexpr auto DPI = 72;
	}

	QFuture<QImage> Document::RenderPage (int num, double xScale, double yScale)
	{
		const std::shared_ptr<Poppler::Page> page (PDocument_->page (num));
		if (!page)
			return Util::MakeReadyFuture (QImage {});

		return QtConcurrent::run ([=] { return page->renderToImage (DPI * xScale, DPI * yScale); });
	}

	QList<ILink_ptr> Document::GetPageLinks (int num)
	{
		QList<ILink_ptr> result;
		std::unique_ptr<Poppler::Page> page (PDocument_->page (num));
		if (!page)
			return result;

		for (const auto link : page->links ())
			result << std::make_shared<Link> (*this, *link);

		return result;
	}

	QUrl Document::GetDocURL () const
	{
		return DocURL_;
	}

	const DocumentSignals* Document::GetDocumentSignals () const
	{
		return &Signals_;
	}

	TOCEntryLevel_t Document::GetTOC ()
	{
		return TOC_;
	}

	QString Document::GetTextContent (int pageNum, const PageRelativeRectBase& rect)
	{
		std::unique_ptr<Poppler::Page> page (PDocument_->page (pageNum));
		if (!page)
			return {};

		const auto& r = rect.ToRectF ();
		const auto& size = page->pageSizeF ();
		const auto w = size.width ();
		const auto h = size.height ();
		return page->text ({ r.x () * w, r.y () * h, r.width () * w, r.height () * h });
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
			return {};

		QList<std::shared_ptr<Poppler::FormField>> popplerFields;
		for (auto field : page->formFields ())
			popplerFields << std::shared_ptr<Poppler::FormField> (field);

		IFormFields_t fields;
		fields.reserve (popplerFields.size ());
		for (const auto& field : popplerFields)
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
			guard = Util::MakeScopeGuard ([this, backend] { PDocument_->setRenderBackend (backend); });
		}

		std::unique_ptr<Poppler::Page> page (PDocument_->page (num));
		if (!page)
			return;

		page->renderToPainter (painter, DPI * xScale, DPI * yScale);
	}

	QMap<int, QList<PageRelativeRectBase>> Document::GetTextPositions (const QString& text, Qt::CaseSensitivity cs)
	{
		QMap<int, QList<PageRelativeRectBase>> result;
		Poppler::Page::SearchFlags searchFlags;
		if (cs != Qt::CaseSensitive)
			searchFlags |= Poppler::Page::SearchFlag::IgnoreCase;

		const auto numPages = PDocument_->numPages ();

		QVector<QList<PageRelativeRectBase>> resVec;
		resVec.resize (numPages);

		auto worker = [&resVec, &searchFlags, &text, this] (int start, int count)
		{
			std::unique_ptr<Poppler::Document> doc (Poppler::Document::load (DocURL_.toLocalFile ()));
			for (auto i = start, end = start + count; i < end; ++i)
			{
				std::unique_ptr<Poppler::Page> p { doc->page (i) };

				const auto& size = p->pageSizeF ();
				const auto scaleMat = QMatrix {}.scale (1 / size.width (), 1 / size.height ());

				resVec [i] = Util::Map (p->search (text, searchFlags),
						[&] (const auto& rect) { return PageRelativeRectBase { scaleMat.mapRect (rect) }; });
			}
		};
		const auto threadCount = QThread::idealThreadCount ();

		std::vector<std::thread> threads;
		threads.reserve (threadCount);
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

		if (path != DocURL_.toLocalFile ())
		{
			conv->setOutputFileName (path);
			return conv->convert ();
		}

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

	void Document::RequestPrinting ()
	{
		emit Signals_.printRequested ({});
	}

	namespace
	{
		TOCEntryLevel_t BuildTOCLevel (const QVector<Poppler::OutlineItem>& level)
		{
			TOCEntryLevel_t result;

			for (const auto& item : level)
			{
				const auto& children = BuildTOCLevel (item.children ());

				NavigationAction navAct;
				if (item.destination ())
					navAct = MakeNavigationAction (*item.destination ());
				else if (!children.isEmpty ())
					navAct = children.value (0).Navigation_;
				else
					continue;

				result.push_back ({
						navAct,
						item.name (),
						children,
					});
			}

			return result;
		}
	}

	void Document::BuildTOC ()
	{
		TOC_ = BuildTOCLevel (PDocument_->outline ());
	}
}
