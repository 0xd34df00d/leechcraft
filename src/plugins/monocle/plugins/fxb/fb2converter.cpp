/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fb2converter.h"
#include <functional>
#include <memory>
#include <QDomDocument>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextFrame>
#include <QImage>
#include <QVariant>
#include <QStack>
#include <QStringList>
#include <QtDebug>
#include <util/sll/either.h>
#include <util/sll/qtutil.h>
#include <util/sll/domchildrenrange.h>
#include <util/sll/timer.h>
#include <util/monocle/textdocumentformatconfig.h>
#include "toclink.h"

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

		const QHash<QStringView, Converter_t> Converters
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

			{ u"style", HtmlTag { .Tag_ = "span"_qs, .Class_ = "style-emphasis"_qs } },

			{ u"cite", HtmlTag { .Tag_ = "blockquote"_qs, .Class_ = "cite-internal"_qs } },
			{ u"poem", HtmlTag { .Tag_ = "div"_qs, .Class_ = "poem"_qs } },
			{ u"stanza", HtmlTag { .Tag_ = "div"_qs, .Class_ = "stanza"_qs } },
			{ u"v", HtmlTag { .Tag_ = "div"_qs } },
		};

		std::optional<HtmlTag> ConvertFb2Tag (const QString& tagName, const QVector<QStringView>& tagStack)
		{
			const auto convIt = Converters.find (tagName);
			if (convIt == Converters.end ())
				return {};

			return Util::Visit (*convIt,
					[] (const HtmlTag& tag) { return tag; },
					[&] (const StackedConverter_t& conv) { return conv (tagStack); });
		}

		void CollapsePs (QDomElement elem)
		{
			auto owner = elem.ownerDocument ();
			bool firstP = true;
			for (auto p = elem.firstChildElement ("p"_qs); !p.isNull (); p = elem.firstChildElement ("p"_qs))
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

		class Converter
		{
			QStack<QStringView> TagStack_;
			QHash<QString, int> UnhandledTags_;
		public:
			auto operator() (const QDomElement& elem)
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

			static void Fixup (QDomElement elem)
			{
				const auto& tagName = elem.tagName ();
				if (tagName == "title"_ql || tagName == "subtitle"_ql || tagName == "epigraph"_qs)
					CollapsePs (elem);

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
			}
		};

		TextDocumentAdapter::ImagesList_t LoadImages (const QDomDocument& fb2)
		{
			TextDocumentAdapter::ImagesList_t images;
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
	}

	ConvertResult_t Convert (QDomDocument&& fb2)
	{
		const auto& fb2root = fb2.documentElement ();
		const auto& body = fb2root.firstChildElement ("body"_qs);
		if (fb2root.tagName () != "FictionBook"_ql || body.isNull ())
		{
			qWarning () << "not a FictionBook document";
			return ConvertResult_t::Left (QObject::tr ("Not a FictionBook document."));
		}

		Util::Timer timer;
		const auto& converted = Converter {} (body);
		timer.Stamp ("converting fb2");
		const auto& binaries = LoadImages (fb2);
		timer.Stamp ("loading images");
		return ConvertResult_t::Right ({ converted, binaries });
	}
}
