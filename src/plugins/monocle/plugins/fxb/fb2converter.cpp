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
	FB2Converter::FB2Converter (const QDomDocument& fb2)
	: FB2_ (fb2)
	, Result_ (new QTextDocument)
	, Cursor_ (new QTextCursor (Result_))
	, SectionLevel_ (0)
	{
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

		Handlers_ ["section"] = [this] (const HandlerParams& p) { HandleSection (p); };
		Handlers_ ["title"] = [this] (const HandlerParams& p) { HandleTitle (p); };
		Handlers_ ["epigraph"] = [this] (const HandlerParams& p) { HandleEpigraph (p); };
		Handlers_ ["image"] = [this] (const HandlerParams& p) { HandleImage (p); };

		Handlers_ ["p"] = [this] (const HandlerParams& p) { HandlePara (p); };
		Handlers_ ["empty-line"] = [this] (const HandlerParams& p) { HandleEmptyLine (p); };

		Handlers_ ["emphasis"] = [this] (const HandlerParams& p)
		{
			HandleMangleCharFormat (p,
					[] (QTextCharFormat& fmt) { fmt.setFontItalic (true); },
					[this] (const HandlerParams& p) { HandleParaWONL (p); });
		};
		Handlers_ ["strong"] = [this] (const HandlerParams& p)
		{
			HandleMangleCharFormat (p,
					[] (QTextCharFormat& fmt) { fmt.setFontWeight (QFont::Bold); },
					[this] (const HandlerParams& p) { HandleParaWONL (p); });
		};
		Handlers_ ["strikethrough"] = [this] (const HandlerParams& p)
		{
			HandleMangleCharFormat (p,
					[] (QTextCharFormat& fmt) { fmt.setFontStrikeOut (true); },
					[this] (const HandlerParams& p) { HandleParaWONL (p); });
		};

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

	void FB2Converter::HandleDescription (const QDomElement& elem)
	{
		auto getChildValues = [&elem] (const QString& nodeName)
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

	struct HandlerParams
	{
		const QDomElement& Elem_;
	};

	void FB2Converter::HandleBody (const QDomElement& bodyElem)
	{
		FillPreamble ();

		auto elem = bodyElem.firstChildElement ();
		while (!elem.isNull ())
		{
			const auto& tagName = elem.tagName ();
			Handlers_.value (tagName, [&tagName] (const HandlerParams&)
					{
						qWarning () << Q_FUNC_INFO
								<< "unhandled tag"
								<< tagName;
					}) ({ elem });
			elem = elem.nextSiblingElement ();
		}
	}

	void FB2Converter::HandleSection (const HandlerParams& params)
	{
		++SectionLevel_;

		auto child = params.Elem_.firstChildElement ();
		while (!child.isNull ())
		{
			const auto& tagName = child.tagName ();
			Handlers_.value (tagName,
					[&tagName] (const HandlerParams&)
					{
						qWarning () << Q_FUNC_INFO
								<< "unhandled tag"
								<< tagName;
					}) ({ child });

			child = child.nextSiblingElement ();
		}

		--SectionLevel_;
	}

	void FB2Converter::HandleTitle (const HandlerParams& params)
	{
		auto topFrame = Cursor_->currentFrame ();

		QTextFrameFormat frameFmt;
		frameFmt.setBorder (1);
		frameFmt.setPadding (10);
		frameFmt.setBackground (QColor ("#A4C0E4"));
		Cursor_->insertFrame (frameFmt);

		auto child = params.Elem_.firstChildElement ();
		while (!child.isNull ())
		{
			const auto& tagName = child.tagName ();

			if (tagName == "empty-line")
				Handlers_ [tagName] ({ child });
			else if (tagName == "p")
			{
				const auto origFmt = Cursor_->charFormat ();

				auto titleFmt = origFmt;
				titleFmt.setFontPointSize (18 - SectionLevel_);
				Cursor_->setCharFormat (titleFmt);

				Handlers_ ["p"] ({ child });

				Cursor_->setCharFormat (origFmt);
			}

			child = child.nextSiblingElement ();
		}

		Cursor_->setPosition (topFrame->lastPosition ());
	}

	void FB2Converter::HandleEpigraph (const HandlerParams& params)
	{
	}

	void FB2Converter::HandleImage (const HandlerParams& params)
	{
	}

	void FB2Converter::HandlePara (const HandlerParams& params)
	{
		auto fmt = Cursor_->blockFormat ();
		fmt.setTextIndent (10);
		Cursor_->setBlockFormat (fmt);

		HandleParaWONL (params);

		Cursor_->insertBlock ();
	}

	void FB2Converter::HandleParaWONL (const HandlerParams& params)
	{
		auto child = params.Elem_.firstChild ();
		while (!child.isNull ())
		{
			std::shared_ptr<void> guard (static_cast<void*> (0),
					[&child] (void*) { child = child.nextSibling (); });

			if (child.isText ())
			{
				Cursor_->insertText (child.toText ().data ());
				continue;
			}

			if (!child.isElement ())
				continue;

			const auto& asElem = child.toElement ();
			const auto& tagName = asElem.tagName ();

			Handlers_.value (tagName, [&tagName] (const HandlerParams&)
					{
						qWarning () << Q_FUNC_INFO
								<< "unhandled tag"
								<< tagName;
					}) ({ asElem });
		}
	}

	void FB2Converter::HandleEmptyLine (const HandlerParams&)
	{
		Cursor_->insertText ("\n\n");
	}

	void FB2Converter::HandleMangleCharFormat (const HandlerParams& params,
			std::function<void (QTextCharFormat&)> mangler, Handler_f next)
	{
		const auto origFmt = Cursor_->charFormat ();

		auto mangledFmt = origFmt;
		mangler (mangledFmt);
		Cursor_->setCharFormat (mangledFmt);

		next ({ params.Elem_ });

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
