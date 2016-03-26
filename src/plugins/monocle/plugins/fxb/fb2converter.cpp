/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "fb2converter.h"
#include <functional>
#include <memory>
#include <QDomDocument>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextFrame>
#include <QUrl>
#include <QImage>
#include <QApplication>
#include <QPalette>
#include <QVariant>
#include <QStringList>
#include <QtDebug>
#include <util/sll/util.h>
#include "toclink.h"

namespace LeechCraft
{
namespace Monocle
{
namespace FXB
{
	class CursorCacher
	{
		QTextCursor * const Cursor_;

		QString Text_;

		QTextBlockFormat LastBlockFormat_ = Cursor_->blockFormat ();
		QTextCharFormat LastCharFormat_ = Cursor_->charFormat ();
	public:
		CursorCacher (QTextCursor *cursor);
		~CursorCacher ();

		const QTextBlockFormat& blockFormat () const;
		const QTextCharFormat& charFormat () const;

		void insertBlock (const QTextBlockFormat&);
		void insertText (const QString&);

		void setCharFormat (const QTextCharFormat&);

		void Flush ();
	};

	CursorCacher::CursorCacher (QTextCursor *cursor)
	: Cursor_ (cursor)
	{
	}

	CursorCacher::~CursorCacher ()
	{
		Flush ();
	}

	const QTextBlockFormat& CursorCacher::blockFormat () const
	{
		return LastBlockFormat_;
	}

	const QTextCharFormat& CursorCacher::charFormat () const
	{
		return LastCharFormat_;
	}

	void CursorCacher::insertBlock (const QTextBlockFormat& fmt)
	{
		if (fmt == LastBlockFormat_)
		{
			if (!Text_.isEmpty ())
				Text_ += "\n";

			return;
		}

		Flush ();

		Cursor_->insertBlock (fmt);
		LastBlockFormat_ = fmt;
	}

	void CursorCacher::insertText (const QString& text)
	{
		Text_ += text;
	}

	void CursorCacher::setCharFormat (const QTextCharFormat& fmt)
	{
		if (LastCharFormat_ == fmt)
			return;

		Flush ();

		Cursor_->setCharFormat (fmt);
		LastCharFormat_ = fmt;
	}

	void CursorCacher::Flush ()
	{
		if (Text_.isEmpty ())
			return;

		Cursor_->insertText (Text_);
		Text_.clear ();
	}

	FB2Converter::FB2Converter (Document *doc, const QDomDocument& fb2)
	: ParentDoc_ (doc)
	, FB2_ (fb2)
	, Result_ (new QTextDocument)
	, Cursor_ (new QTextCursor (Result_))
	, CursorCacher_ (new CursorCacher (Cursor_))
	{
		Result_->setPageSize (QSize (600, 800));
		Result_->setUndoRedoEnabled (false);

		const auto& docElem = FB2_.documentElement ();
		if (docElem.tagName () != "FictionBook")
		{
			Error_ = tr ("Invalid FictionBook document.");
			return;
		}

		const auto rootFrame = Result_->rootFrame ();

		auto frameFmt = rootFrame->frameFormat ();
		frameFmt.setMargin (20);
		const auto& pal = qApp->palette ();
		frameFmt.setBackground (pal.brush (QPalette::Base));
		rootFrame->setFrameFormat (frameFmt);

		Handlers_ ["section"] = [this] (const QDomElement& p) { HandleSection (p); };
		Handlers_ ["title"] = [this] (const QDomElement& p) { HandleTitle (p); };
		Handlers_ ["subtitle"] = [this] (const QDomElement& p) { HandleTitle (p, 1); };
		Handlers_ ["epigraph"] = [this] (const QDomElement& p) { HandleEpigraph (p); };
		Handlers_ ["image"] = [this] (const QDomElement& p) { HandleImage (p); };

		Handlers_ ["p"] = [this] (const QDomElement& p) { HandlePara (p); };
		Handlers_ ["poem"] = [this] (const QDomElement& p) { HandlePoem (p); };
		Handlers_ ["empty-line"] = [this] (const QDomElement& p) { HandleEmptyLine (p); };
		Handlers_ ["stanza"] = [this] (const QDomElement& p) { HandleStanza (p); };
		Handlers_ ["v"] = [this] (const QDomElement& p)
		{
			auto blockFmt = CursorCacher_->blockFormat ();
			blockFmt.setTextIndent (50);

			CursorCacher_->insertBlock (blockFmt);

			HandleMangleCharFormat (p,
					[] (QTextCharFormat& fmt) { fmt.setFontItalic (true); },
					[this] (const QDomElement& p) { HandleParaWONL (p); });
		};

		Handlers_ ["emphasis"] = [this] (const QDomElement& p)
		{
			HandleMangleCharFormat (p,
					[] (QTextCharFormat& fmt) { fmt.setFontItalic (true); },
					[this] (const QDomElement& p) { HandleParaWONL (p); });
		};
		Handlers_ ["strong"] = [this] (const QDomElement& p)
		{
			HandleMangleCharFormat (p,
					[] (QTextCharFormat& fmt) { fmt.setFontWeight (QFont::Bold); },
					[this] (const QDomElement& p) { HandleParaWONL (p); });
		};
		Handlers_ ["strikethrough"] = [this] (const QDomElement& p)
		{
			HandleMangleCharFormat (p,
					[] (QTextCharFormat& fmt) { fmt.setFontStrikeOut (true); },
					[this] (const QDomElement& p) { HandleParaWONL (p); });
		};

		Handlers_ ["annotation"] = [this] (const QDomElement& p)
		{
			HandleMangleBlockFormat (p,
					[] (QTextBlockFormat& fmt)
					{
						fmt.setAlignment (Qt::AlignRight);
						fmt.setLeftMargin (60);
					},
					[this] (const QDomElement& p)
					{
						HandleMangleCharFormat (p,
								[] (QTextCharFormat& fmt) { fmt.setFontItalic (true); },
								[this] (const QDomElement& p) { HandleChildren (p); });
					});
		};
		Handlers_ ["style"] = [this] (const QDomElement& p) { HandleParaWONL (p); };
		Handlers_ ["coverpage"] = [this] (const QDomElement& p) { HandleChildren (p); };

		TOCEntry entry =
		{
			ILink_ptr (),
			"root",
			TOCEntryLevel_t ()
		};
		CurrentTOCStack_.push (&entry);

		auto elem = docElem.firstChildElement ();
		while (!elem.isNull ())
		{
			const auto& tagName = elem.tagName ();
			if (tagName == "description")
				HandleDescription (elem);
			else if (tagName == "body")
				HandleBody (elem);

			elem = elem.nextSiblingElement ();
		}

		TOC_ = entry.ChildLevel_;

		CursorCacher_->Flush ();
	}

	FB2Converter::~FB2Converter ()
	{
		delete CursorCacher_;
		delete Cursor_;
	}

	QString FB2Converter::GetError () const
	{
		return Error_;
	}

	QTextDocument* FB2Converter::GetResult () const
	{
		return Result_;
	}

	DocumentInfo FB2Converter::GetDocumentInfo () const
	{
		return DocInfo_;
	}

	TOCEntryLevel_t FB2Converter::GetTOC () const
	{
		return TOC_;
	}

	QDomElement FB2Converter::FindBinary (const QString& refId) const
	{
		const auto& binaries = FB2_.elementsByTagName ("binary");
		for (int i = 0; i < binaries.size (); ++i)
		{
			const auto& elem = binaries.at (i).toElement ();
			if (elem.attribute ("id") == refId)
				return elem;
		}

		return {};
	}

	void FB2Converter::HandleDescription (const QDomElement& elem)
	{
		QStringList handledChildren;
		auto getChildValues = [&elem, &handledChildren] (const QString& nodeName) -> QStringList
		{
			handledChildren << nodeName;

			QStringList result;
			auto elems = elem.elementsByTagName (nodeName);
			for (int i = 0; i < elems.size (); ++i)
			{
				const auto& elem = elems.at (i);
				const auto& str = elem.toElement ().text ();
				if (!str.isEmpty ())
					result << str;
			}
			return result;
		};

		DocInfo_.Genres_ = getChildValues ("genre");
		DocInfo_.Title_ = getChildValues ("book-title").value (0);
		DocInfo_.Keywords_ = getChildValues ("keywords")
				.value (0).split (' ', QString::SkipEmptyParts);

		const auto& dateElem = elem.elementsByTagName ("date").at (0).toElement ();
		DocInfo_.Date_.setDate (QDate::fromString (dateElem.attribute ("value"), Qt::ISODate));

		DocInfo_.Author_ += getChildValues ("first-name").value (0) + " ";
		DocInfo_.Author_ += getChildValues ("last-name").value (0) + " ";
		const auto& email = getChildValues ("email").value (0);
		if (!email.isEmpty ())
			DocInfo_.Author_ += "<" + email + "> ";
		DocInfo_.Author_ += getChildValues ("nickname").value (0);

		DocInfo_.Author_ = DocInfo_.Author_.trimmed ().simplified ();

		FillPreamble ();

		const auto& titleInfo = elem.firstChildElement ("title-info");

		auto childElem = titleInfo.firstChildElement ();
		while (!childElem.isNull ())
		{
			if (!handledChildren.contains (childElem.tagName ()))
				Handle (childElem);
			childElem = childElem.nextSiblingElement ();
		}
	}

	void FB2Converter::HandleBody (const QDomElement& bodyElem)
	{
		HandleChildren (bodyElem);
	}

	void FB2Converter::HandleSection (const QDomElement& tagElem)
	{
		++SectionLevel_;

		CurrentTOCStack_.top ()->ChildLevel_.append (TOCEntry ());
		CurrentTOCStack_.push (&CurrentTOCStack_.top ()->ChildLevel_.last ());

		auto child = tagElem.firstChildElement ();
		while (!child.isNull ())
		{
			Handle (child);
			child = child.nextSiblingElement ();
		}

		CurrentTOCStack_.pop ();

		--SectionLevel_;
	}

	void FB2Converter::HandleTitle (const QDomElement& tagElem, int level)
	{
		if (CurrentTOCStack_.top ()->Name_.isEmpty ())
		{
			auto child = tagElem.firstChildElement ();
			while (!child.isNull ())
			{
				if (child.tagName () == "p")
				{
					*CurrentTOCStack_.top () = TOCEntry
							{
								std::make_shared<TOCLink> (ParentDoc_, Result_->pageCount () - 1),
								child.text (),
								{}
							};
					break;
				}

				child = child.nextSiblingElement ();
			}
		}

		const auto currentSectionLevel = SectionLevel_;
		HandleMangleBlockFormat (tagElem,
				[] (QTextBlockFormat&) {},
				[=] (const QDomElement& e)
				{
					HandleMangleCharFormat (e,
							[=] (QTextCharFormat& fmt)
							{
								const auto newSize = 18 - 2 * level - currentSectionLevel;
								fmt.setFontPointSize (newSize);
								fmt.setFontWeight (currentSectionLevel <= 1 ? QFont::Bold : QFont::DemiBold);
							},
							[this] (const QDomElement& e) { HandleChildren (e); });
				});
	}

	void FB2Converter::HandleEpigraph (const QDomElement& tagElem)
	{
		auto child = tagElem.firstChildElement ();
		while (!child.isNull ())
		{
			Handle (child);
			child = child.nextSiblingElement ();
		}
	}

	void FB2Converter::HandleImage (const QDomElement& imageElem)
	{
		const auto& refId = imageElem.attribute ("href").mid (1);
		const auto& binary = FindBinary (refId);
		const auto& imageData = QByteArray::fromBase64 (binary.text ().toLatin1 ());
		const auto& image = QImage::fromData (imageData);

		Result_->addResource (QTextDocument::ImageResource,
				{ "image://" + refId },
				QVariant::fromValue (image));

		CursorCacher_->Flush ();
		Cursor_->insertHtml (QString ("<img src='image://%1'/>").arg (refId));
	}

	void FB2Converter::HandlePara (const QDomElement& tagElem)
	{
		auto fmt = CursorCacher_->blockFormat ();
		fmt.setTextIndent (20);
		fmt.setAlignment (Qt::AlignJustify);
		CursorCacher_->insertBlock (fmt);

		HandleParaWONL (tagElem);
	}

	void FB2Converter::HandleParaWONL (const QDomElement& tagElem)
	{
		auto child = tagElem.firstChild ();
		while (!child.isNull ())
		{
			const auto guard = Util::MakeScopeGuard ([&child] { child = child.nextSibling (); });

			if (child.isText ())
			{
				CursorCacher_->insertText (child.toText ().data ());
				continue;
			}

			if (!child.isElement ())
				continue;

			Handle (child.toElement ());
		}
	}

	void FB2Converter::HandlePoem (const QDomElement& tagElem)
	{
		HandleChildren (tagElem);
	}

	void FB2Converter::HandleStanza (const QDomElement& tagElem)
	{
		HandleChildren (tagElem);
	}

	void FB2Converter::HandleEmptyLine (const QDomElement&)
	{
		CursorCacher_->insertText ("\n\n");
	}

	void FB2Converter::HandleChildren (const QDomElement& tagElem)
	{
		auto child = tagElem.firstChildElement ();
		while (!child.isNull ())
		{
			Handle (child);
			child = child.nextSiblingElement ();
		}
	}

	void FB2Converter::Handle (const QDomElement& child)
	{
		const auto& tagName = child.tagName ();
		Handlers_.value (tagName, [&tagName] (const QDomElement&)
					{
						qWarning () << Q_FUNC_INFO
								<< "unhandled tag"
								<< tagName;
					}) (child);
	}

	void FB2Converter::HandleMangleBlockFormat (const QDomElement& tagElem,
			std::function<void (QTextBlockFormat&)> mangler, Handler_f next)
	{
		const auto origFmt = CursorCacher_->blockFormat ();

		auto mangledFmt = origFmt;
		mangler (mangledFmt);
		CursorCacher_->insertBlock (mangledFmt);

		next ({ tagElem });

		CursorCacher_->insertBlock (origFmt);
	}

	void FB2Converter::HandleMangleCharFormat (const QDomElement& tagElem,
			std::function<void (QTextCharFormat&)> mangler, Handler_f next)
	{
		const auto origFmt = CursorCacher_->charFormat ();

		auto mangledFmt = origFmt;
		mangler (mangledFmt);
		CursorCacher_->setCharFormat (mangledFmt);

		next ({ tagElem });

		CursorCacher_->setCharFormat (origFmt);
	}

	void FB2Converter::FillPreamble ()
	{
		CursorCacher_->Flush ();

		auto topFrame = Cursor_->currentFrame ();

		QTextFrameFormat format;
		format.setBorder (2);
		format.setPadding (8);
		format.setBackground (QColor ("#6193CF"));

		if (!DocInfo_.Title_.isEmpty ())
		{
			Cursor_->insertFrame (format);
			QTextCharFormat charFmt;
			charFmt.setFontPointSize (18);
			charFmt.setFontWeight (QFont::Bold);
			Cursor_->insertText (DocInfo_.Title_, charFmt);

			Cursor_->setPosition (topFrame->lastPosition ());
		}

		if (!DocInfo_.Author_.isEmpty ())
		{
			format.setBorder (1);
			Cursor_->insertFrame (format);

			QTextCharFormat charFmt;
			charFmt.setFontPointSize (12);
			charFmt.setFontItalic (true);
			Cursor_->insertText (DocInfo_.Author_, charFmt);

			Cursor_->setPosition (topFrame->lastPosition ());
		}

		Cursor_->insertBlock ();
	}
}
}
}
