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
#include <QDomDocument>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextFrame>
#include <QUrl>
#include <QImage>
#include <QVariant>
#include <QStringList>
#include <boost-1_49/boost/concept_check.hpp>

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

	void FB2Converter::HandleBody (const QDomElement& elem)
	{
		FillPreamble ();
	}

	void FB2Converter::FillPreamble ()
	{
		auto topFrame = Cursor_->currentFrame ();

		QTextFrameFormat format;
		format.setBorder (2);
		format.setPadding (8);
		format.setBackground (QColor ("#A4C0E4"));

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
