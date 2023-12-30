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
#include <util/sll/util.h>
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

			{ u"coverpage", HtmlTag { .Tag_ ="div"_qs, .Class_ = "break-after" } },

			{ u"section", HtmlTag { .Tag_ = "div"_qs, .Class_ = "section"_qs } },
			{ u"title", &StackedTitle<1> },
			{ u"subtitle", &StackedTitle<2> },
			Identity<u"p"> (),
			{ u"epigraph", HtmlTag { .Tag_ = "div"_qs, .Class_ = "epigraph"_qs } },

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
	}

	std::optional<HtmlTag> ConvertFb2Tag (const QString& tagName, const QVector<QStringView>& tagStack)
	{
		const auto convIt = Converters.find (tagName);
		if (convIt == Converters.end ())
			return {};

		return Util::Visit (*convIt,
				[] (const HtmlTag& tag) { return tag; },
				[&] (const StackedConverter_t& conv) { return conv (tagStack); });
	}

	namespace
	{
		QString ToString (Qt::AlignmentFlag flag)
		{
			switch (flag)
			{
			case Qt::AlignLeft:
				return "left"_qs;
			case Qt::AlignRight:
				return "right"_qs;
			case Qt::AlignHCenter:
				return "center"_qs;
			case Qt::AlignJustify:
				return "justify"_qs;
			default:
				break;
			}

			return {};
		}

		std::optional<int> GetHeaderLevel (const QString& tagName)
		{
			if (tagName.size () != 2 || tagName [0] != 'h')
				return {};

			auto level = tagName [1].toLatin1 () - '0';
			if (level < 1 || level > 6)
				return {};
			return level;
		}

		class Converter
		{
			QStack<QStringView> TagStack_;
			QHash<QString, int> UnhandledTags_;
			NonStyleSheetStyles Styles_ = TextDocumentFormatConfig::Instance ().GetNonStyleSheetStyles ();
		public:
			QString operator() (const QDomElement& elem)
			{
				Util::Timer timer;
				RunConvert (elem);
				timer.Stamp ("fb2 conversion");

				if (!UnhandledTags_.isEmpty ())
					qWarning () << "unhandled tags:" << UnhandledTags_;

				QString html;
				QTextStream htmlStream { &html };
				elem.save (htmlStream, 0);
				timer.Stamp ("html serialization");
				return html;
			}
		private:
			void RunConvert (QDomElement elem)
			{
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

				Fixup (elem);
			}

			void Fixup (QDomElement elem)
			{
				static const QSet overriddenAlignment
				{
					u"epigraph"_qsv,
					u"cite"_qsv,
					u"poem"_qsv,
					u"stanza"_qsv
				};
				if (elem.tagName () == "p"_ql &&
						!std::any_of (TagStack_.begin (), TagStack_.end (),
								[&] (auto tag) { return overriddenAlignment.contains (tag); }))
					elem.setAttribute ("align"_qs, ToString (Styles_.AlignP_));

				if (const auto headerLevel = GetHeaderLevel (elem.tagName ()))
				{
					elem.setAttribute ("align"_qs, ToString (Styles_.AlignH_ [*headerLevel]));

					auto owner = elem.ownerDocument ();
					bool firstP = true;
					for (auto p = elem.firstChildElement ("p"_qs); !p.isNull (); p = elem.firstChildElement ("p"_qs))
					{
						auto subst = owner.createDocumentFragment ();
						if (!firstP)
							subst.appendChild (owner.createElement ("br"_qs));
						firstP = false;

						const auto grandChildren = p.childNodes ();
						for (int i = 0; i < grandChildren.size (); ++i)
							subst.appendChild (grandChildren.at (i));

						elem.replaceChild (subst, p);
					}
				}
			}
		};

		void CreateLinks (const QTextDocument& doc)
		{
			for (auto block = doc.begin (), end = doc.end (); block != end; block = block.next ())
			{
				auto names = block.charFormat ().anchorNames ();
				if (!names.isEmpty ())
					qDebug () << names;
			}
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

		const auto& html = Converter {} (body);

		Util::Timer timer;
		auto doc = std::make_unique<QTextDocument> ();
		doc->setDefaultStyleSheet (TextDocumentFormatConfig::Instance ().GetStyleSheet ());
		doc->setHtml (html);
		TextDocumentFormatConfig::Instance ().FormatDocument (*doc);
		timer.Stamp ("doc creation");

		CreateLinks (*doc);

		timer.Stamp ("links creation");

		return ConvertResult_t::Right ({ std::move (doc) });
	}
}
