/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "fb2converter.h"
#include "toclink.h"
#include <functional>
#include <memory>
#include <QDomDocument>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextFrame>
#include <QUrl>
#include <QImage>
#include <QVariant>
#include <QStringList>
#include <QtDebug>

namespace LeechCraft
{
namespace Monocle
{
namespace FXB
{
	FB2Converter::FB2Converter (Document *doc, const QDomDocument& fb2)
	: ParentDoc_ (doc)
	, FB2_ (fb2)
	, Result_ (new QTextDocument)
	, Cursor_ (new QTextCursor (Result_))
	, SectionLevel_ (0)
	{
		Result_->setPageSize (QSize (600, 800));

		const auto& docElem = FB2_.documentElement ();
		if (docElem.tagName () != "FictionBook")
		{
			Error_ = tr ("Invalid FictionBook document.");
			return;
		}

		auto elems = docElem.elementsByTagName ("binary");
		for (int i = 0, size = elems.size (); i < size; ++i)
		{
			const auto& elem = elems.at (i).toElement ();
			AddImage (elem);
		}

		auto frameFmt = Result_->rootFrame ()->frameFormat ();
		frameFmt.setMargin (20);
		Result_->rootFrame ()->setFrameFormat (frameFmt);

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
			auto fmt = Cursor_->blockFormat ();
			fmt.setTextIndent (50);
			Cursor_->insertBlock (fmt);
			HandleParaWONL (p);
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

		Handlers_ ["style"] = [this] (const QDomElement& p) { HandleParaWONL (p); };

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
	}

	FB2Converter::~FB2Converter ()
	{
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

	void FB2Converter::HandleDescription (const QDomElement& elem)
	{
		auto getChildValues = [&elem] (const QString& nodeName) -> QStringList
		{
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

		DocInfo_.Author_ = QString ("%1 %2 %3 <%4> %5")
				.arg (getChildValues ("first-name").value (0))
				.arg (getChildValues ("middle-name").value (0))
				.arg (getChildValues ("last-name").value (0))
				.arg (getChildValues ("email").value (0))
				.arg (getChildValues ("nickname").value (0))
				.simplified ();
	}

	void FB2Converter::HandleBody (const QDomElement& bodyElem)
	{
		FillPreamble ();

		auto elem = bodyElem.firstChildElement ();
		while (!elem.isNull ())
		{
			Handle (elem);
			elem = elem.nextSiblingElement ();
		}
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
		auto topFrame = Cursor_->currentFrame ();

		QTextFrameFormat frameFmt;
		frameFmt.setBorder (1);
		frameFmt.setPadding (10);
		frameFmt.setBackground (QColor ("#A4C0E4"));
		Cursor_->insertFrame (frameFmt);

		auto child = tagElem.firstChildElement ();
		while (!child.isNull ())
		{
			const auto& tagName = child.tagName ();

			if (tagName == "empty-line")
				Handlers_ [tagName] ({ child });
			else if (tagName == "p")
			{
				const auto origFmt = Cursor_->charFormat ();

				auto titleFmt = origFmt;
				titleFmt.setFontPointSize (18 - 2 * level - SectionLevel_);
				Cursor_->setCharFormat (titleFmt);

				Handlers_ ["p"] ({ child });

				Cursor_->setCharFormat (origFmt);

				const TOCEntry entry =
				{
					ILink_ptr (new TOCLink (ParentDoc_, Result_->pageCount () - 1)),
					child.text (),
					TOCEntryLevel_t ()
				};
				if (CurrentTOCStack_.top ()->Name_.isEmpty ())
					*CurrentTOCStack_.top () = entry;
			}

			child = child.nextSiblingElement ();
		}

		Cursor_->setPosition (topFrame->lastPosition ());
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

	void FB2Converter::HandleImage (const QDomElement&)
	{
		// TODO
	}

	void FB2Converter::HandlePara (const QDomElement& tagElem)
	{
		auto fmt = Cursor_->blockFormat ();
		fmt.setTextIndent (20);
		Cursor_->setBlockFormat (fmt);

		HandleParaWONL (tagElem);

		Cursor_->insertBlock ();
	}

	void FB2Converter::HandleParaWONL (const QDomElement& tagElem)
	{
		auto child = tagElem.firstChild ();
		while (!child.isNull ())
		{
			std::shared_ptr<void> guard (static_cast<void*> (0),
					[&child] (void*) { child = child.nextSibling (); });

			if (child.isText ())
			{
				auto fmt = Cursor_->charFormat ();
				auto newFmt = fmt;
				newFmt.setForeground (Qt::black);
				Cursor_->setCharFormat (newFmt);

				Cursor_->insertText (child.toText ().data ());

				Cursor_->setCharFormat (fmt);

				continue;
			}

			if (!child.isElement ())
				continue;

			Handle (child.toElement ());
		}
	}

	void FB2Converter::HandlePoem (const QDomElement& tagElem)
	{
		auto child = tagElem.firstChildElement ();
		while (!child.isNull ())
		{
			Handle (child);
			child = child.nextSiblingElement ();
		}
	}

	void FB2Converter::HandleStanza (const QDomElement& tagElem)
	{
		auto child = tagElem.firstChildElement ();
		while (!child.isNull ())
		{
			Handle (child);
			child = child.nextSiblingElement ();
		}
	}

	void FB2Converter::HandleEmptyLine (const QDomElement&)
	{
		Cursor_->insertText ("\n\n");
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

	void FB2Converter::HandleMangleCharFormat (const QDomElement& tagElem,
			std::function<void (QTextCharFormat&)> mangler, Handler_f next)
	{
		const auto origFmt = Cursor_->charFormat ();

		auto mangledFmt = origFmt;
		mangler (mangledFmt);
		Cursor_->setCharFormat (mangledFmt);

		next ({ tagElem });

		Cursor_->setCharFormat (origFmt);
	}

	void FB2Converter::FillPreamble ()
	{
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

	void FB2Converter::AddImage (const QDomElement& elem)
	{
		const auto& data = elem.firstChild ().toText ().data ().toLatin1 ();
		Result_->addResource (QTextDocument::ImageResource,
				QUrl (elem.attribute ("id")),
				QImage::fromData (QByteArray::fromBase64 (data)));
	}
}
}
}
