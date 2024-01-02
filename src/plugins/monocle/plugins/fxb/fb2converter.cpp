/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fb2converter.h"
#include <functional>
#include <QDomDocument>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextFrame>
#include <QImage>
#include <QVariant>
#include <QStack>
#include <QtDebug>
#include <util/sll/either.h>
#include <util/sll/qtutil.h>
#include <util/sll/domchildrenrange.h>
#include <util/sll/timer.h>

namespace LC::Monocle::FXB
{
	struct HtmlTag
	{
		QString Tag_;
		QString Class_ {};
		QString Id_ {};
	};

	namespace
	{
		using StackedConverter_t = std::function<HtmlTag (const QVector<QStringView>&)>;
		using Converter_t = std::variant<HtmlTag, StackedConverter_t>;

		template<Util::CtString Str>
		std::pair<QStringView, Converter_t> Identity ()
		{
			return { Str, HtmlTag { .Tag_ = Util::ToString<Str> () } };
		}

		template<int Base>
		HtmlTag StackedTitle (const QVector<QStringView>& stack)
		{
			const auto depth = stack.count (u"section"_qsv) + stack.count (u"subsection"_qsv);
			const auto maxH = 6;
			return HtmlTag { .Tag_ = "h" + QString::number (std::min (Base + depth, maxH)) };
		}

		std::optional<HtmlTag> ConvertFb2Tag (const QString& tagName, const QVector<QStringView>& tagStack)
		{
			static const QHash<QStringView, Converter_t> converters
			{
				Identity<u"body"> (),

				{ u"section", HtmlTag { .Tag_ = "div"_qs, .Class_ = "section"_qs } },
				{ u"title", &StackedTitle<1> },
				{ u"subtitle", &StackedTitle<2> },
				Identity<u"p"> (),
				{ u"epigraph", HtmlTag { .Tag_ = "div"_qs, .Class_ = "epigraph"_qs } },
				{ u"image", HtmlTag { .Tag_ = "img"_qs } },

				{ u"empty-line", HtmlTag { .Tag_ = "br"_qs } },

				{ u"emphasis", HtmlTag { .Tag_ = "em"_qs } },
				Identity<u"strong"> (),
				{ u"strikethrough", HtmlTag { .Tag_ = "s"_qs } },
				Identity<u"a"> (),

				{ u"style", HtmlTag { .Tag_ = "span"_qs, .Class_ = "style-emphasis"_qs } },

				{ u"cite", HtmlTag { .Tag_ = "blockquote"_qs, .Class_ = "cite-internal"_qs } },
				{ u"poem", HtmlTag { .Tag_ = "div"_qs, .Class_ = "poem"_qs } },
				{ u"stanza", HtmlTag { .Tag_ = "div"_qs, .Class_ = "stanza"_qs } },
			};
			const auto convIt = converters.find (tagName);
			if (convIt == converters.end ())
				return {};

			return Util::Visit (*convIt,
					[] (const HtmlTag& tag) { return tag; },
					[&] (const StackedConverter_t& conv) { return conv (tagStack); });
		}

		void CollapseChildren (QDomElement elem, const QString& childName)
		{
			auto owner = elem.ownerDocument ();
			bool firstP = true;
			for (auto p = elem.firstChildElement (childName); !p.isNull (); p = elem.firstChildElement (childName))
			{
				auto subst = owner.createDocumentFragment ();
				if (!firstP)
					subst.appendChild (owner.createTextNode ({ '\n' }));
				firstP = false;

				const auto grandChildren = p.childNodes ();
				for (int i = 0; i < grandChildren.size (); ++i)
					subst.appendChild (grandChildren.at (i));

				elem.replaceChild (subst, p);
			}
		}

		QString ExtractTitle (const QDomElement& titleElem)
		{
			const auto& p = titleElem.firstChildElement ("p"_qs);
			return !p.isNull () ? p.text () : titleElem.text ();
		}

		void MarkSection (QDomElement section)
		{
			QString sectionTitle;

			const auto& title = section.firstChildElement ("title"_qs);
			if (!title.isNull ())
				sectionTitle = ExtractTitle (title);

			if (sectionTitle.isEmpty ())
			{
				constexpr auto sectionNameSize = 50;
				sectionTitle = section.text ();
				if (sectionTitle.size () > sectionNameSize)
				{
					sectionTitle.truncate (sectionNameSize);
					sectionTitle += u"…"_qs;
				}
			}

			section.setAttribute ("section-title"_qs, sectionTitle);
		}

		void Fixup (QDomElement& elem)
		{
			const auto& tagName = elem.tagName ();
			if (tagName == "title"_ql || tagName == "subtitle"_ql || tagName == "epigraph"_qs)
				CollapseChildren (elem, "p"_qs);

			if (tagName == "stanza"_ql)
				CollapseChildren (elem, "v"_qs);

			if (tagName == "section"_ql || tagName == "subsection"_ql)
				MarkSection (elem);

			if (tagName == "image"_ql)
			{
				const auto& refId = elem.attribute ("href"_qs);
				if (refId.isEmpty () || refId [0] != '#')
				{
					qWarning () << "unknown ref id"
							<< refId;
					return;
				}

				elem.removeAttribute ("href"_qs);
				elem.setAttribute ("src"_qs, refId.mid (1));
			}

			if (tagName == "a"_ql && elem.attribute ("type"_qs) == "note"_ql)
			{
				auto parent = elem.parentNode ().toElement ();
				if (parent.tagName () != "sup"_qs)
				{
					auto a = elem;
					elem = elem.ownerDocument ().createElement ("sup"_qs);
					elem.appendChild (a);
					parent.appendChild (elem);
				}
			}
		}

		class Converter
		{
			QStack<QStringView> TagStack_;
			QHash<QString, int> UnhandledTags_;
		public:
			auto operator() (QDomElement&& elem)
			{
				RunConvert (elem);

				if (!UnhandledTags_.isEmpty ())
					qWarning () << "unhandled tags:" << UnhandledTags_;

				return elem;
			}
		private:
			void RunConvert (QDomElement elem)
			{
				Fixup (elem);

				const auto& fb2tag = elem.tagName ();
				for (const auto& child : Util::DomChildren (elem, {}))
				{
					TagStack_.push (fb2tag);
					RunConvert (child);
					TagStack_.pop ();
				}

				const auto& htmlTag = ConvertFb2Tag (fb2tag, TagStack_);
				if (!htmlTag)
				{
					++UnhandledTags_ [fb2tag];
					return;
				}

				if (htmlTag->Tag_ != fb2tag)
					elem.setTagName (htmlTag->Tag_);
				if (!htmlTag->Class_.isEmpty ())
					elem.setAttribute ("class"_qs, htmlTag->Class_);
			}
		};

		ImagesList_t LoadImages (const QDomDocument& fb2)
		{
			ImagesList_t images;
			const auto& binaries = fb2.elementsByTagName ("binary"_qs);
			images.reserve (binaries.size ());
			for (int i = 0; i < binaries.size (); ++i)
			{
				const auto& binary = binaries.at (i).toElement ();
				const auto& contentType = binary.attribute ("content-type"_qs);
				if (!contentType.startsWith ("image/"_ql))
				{
					qWarning () << "unsupported content-type" << contentType;
					continue;
				}

				const auto& imageData = QByteArray::fromBase64 (binary.text ().toLatin1 ());
				const auto& image = QImage::fromData (imageData);
				const auto& id = binary.attribute ("id"_qs);
				images.push_back ({ id, image });
			}

			return images;
		}

		QString GetCoverImageId (const QDomDocument& doc)
		{
			return doc
					.elementsByTagName ("coverpage"_qs).at (0).toElement ()
					.elementsByTagName ("image"_qs).at (0).toElement ()
					.attribute ("href"_qs).mid (1);
		}

		struct Notes
		{
			QDomElement NotesPage_;
			QHash<QString, QString> Id2Text_;
		};

		QString ExtractNoteText (const QDomElement& noteSection)
		{
			auto textFragment = QDomDocument {}.createDocumentFragment ();

			Converter cvt;
			for (const auto& p : Util::DomChildren (noteSection, "p"_qs))
				textFragment.appendChild (cvt (p.cloneNode ().toElement ()));

			QString html;
			QTextStream htmlStream { &html };
			textFragment.save (htmlStream, 0);
			return html;
		}

		std::optional<Notes> GetNotes (QDomElement&& notesBody)
		{
			QHash<QString, QString> structured;
			for (const auto& noteSection : Util::DomChildren (notesBody, "section"_qs))
			{
				const auto& id = noteSection.attribute ("id"_qs);
				const auto& text = ExtractNoteText (noteSection);
				if (!id.isEmpty () && !text.isEmpty ())
					structured [id] = text;
			}

			auto elem = Converter {} (std::move (notesBody));
			return Notes { .NotesPage_ = elem, .Id2Text_ = structured };
		}

		void AppendNotes (QDomElement& body, const Notes& notes)
		{
			auto notesNodes = notes.NotesPage_.childNodes ();
			while (!notesNodes.isEmpty ())
				body.appendChild (notesNodes.at (0));

			const auto& links = body.elementsByTagName ("a"_qs);
			for (int i = 0; i < links.size (); ++i)
			{
				auto link = links.at (i).toElement ();
				auto href = link.attribute ("href"_qs);
				if (href.startsWith ('#'))
					href = href.mid (1);

				const auto& text = notes.Id2Text_.value (href);
				if (!text.isEmpty ())
					link.setAttribute ("title"_qs, text);
			}
		}
	}

	ConvertResult_t Convert (QDomDocument&& fb2)
	{
		const auto& fb2root = fb2.documentElement ();
		auto fb2body = fb2root.firstChildElement ("body"_qs);
		if (fb2root.tagName () != "FictionBook"_ql || fb2body.isNull ())
		{
			qWarning () << "not a FictionBook document";
			return ConvertResult_t::Left (QObject::tr ("Not a FictionBook document."));
		}

		auto notesElem = fb2body.nextSiblingElement ("body"_qs);

		Util::Timer timer;
		auto body = Converter {} (std::move (fb2body));
		timer.Stamp ("converting fb2");

		auto binaries = LoadImages (fb2);
		timer.Stamp ("loading images");

		if (auto notes = GetNotes (std::move (notesElem)))
			AppendNotes (body, *notes);
		timer.Stamp ("converting notes and comments");

		return ConvertResult_t::Right ({ body, std::move (binaries), GetCoverImageId (fb2) });
	}
}
