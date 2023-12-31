/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "html2doc.h"
#include <QDomElement>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFrame>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include "textdocumentformatconfig.h"

namespace LC::Monocle
{
	namespace
	{
		bool IsBlockElem (QStringView tagName)
		{
			static const QSet blockElems
			{
				u"div"_qsv,
				u"p"_qsv,
				u"h1"_qsv,
				u"h2"_qsv,
				u"h3"_qsv,
				u"h4"_qsv,
				u"h5"_qsv,
				u"h6"_qsv,
			};
			return blockElems.contains (tagName);
		}

		template<typename T>
		class FormatKeeper final
		{
			T& Format_;
			const T Saved_;
			bool NeedRestore_ = false;
		public:
			FormatKeeper (T& toSave)
			: Format_ { toSave }
			, Saved_ { toSave }
			{
			}

			~FormatKeeper ()
			{
				if (NeedRestore_)
					Format_ = Saved_;
			}

			void SetFormat (T&& format)
			{
				Format_ = std::move (format);
				NeedRestore_ = true;
			}

			FormatKeeper (const FormatKeeper&) = delete;
			FormatKeeper (FormatKeeper&) = delete;
			FormatKeeper& operator= (const FormatKeeper&) = delete;
			FormatKeeper& operator= (FormatKeeper&) = delete;
		};

		class Converter
		{
			const TextDocumentFormatConfig& Config_ = TextDocumentFormatConfig::Instance ();

			QTextDocument& Doc_;
			QTextFrame& BodyFrame_;
			QTextCursor Cursor_;

			QTextCharFormat CharFormat_;
		public:
			explicit Converter (QTextDocument& doc)
			: Doc_ { doc }
			, BodyFrame_ { *QTextCursor { &doc }.insertFrame (Config_.GetBodyFrameFormat ()) }
			, Cursor_ { &BodyFrame_ }
			{
				auto rootFmt = Doc_.rootFrame ()->frameFormat ();
				rootFmt.setBackground (Config_.GetPalette ().Background_);
				Doc_.rootFrame ()->setFrameFormat (rootFmt);
			}

			void operator() (const QDomElement& body)
			{
				AppendElem (body);
			}
		private:
			void AppendElem (const QDomElement& elem)
			{
				FormatKeeper charKeeper { CharFormat_ };
				if (auto maybeCharFmt = GetElemCharFormat (elem))
					charKeeper.SetFormat (*std::move (maybeCharFmt));

				if (IsBlockElem (elem.tagName ()))
					Cursor_.insertBlock (GetElemBlockFormat (elem), CharFormat_);

				HandleElem (elem);
				HandleChildren (elem);
			}

			void AppendText (const QDomText& textNode)
			{
				Cursor_.insertText (textNode.data (), CharFormat_);
			}

			void HandleElem (const QDomElement& elem)
			{
				if (elem.tagName () == "br"_ql)
					Cursor_.insertText (QString { '\n' });
			}

			void HandleChildren (const QDomElement& elem)
			{
				const auto& children = elem.childNodes ();
				for (int i = 0; i < children.size (); ++i)
				{
					const auto& child = children.at (i);
					switch (child.nodeType ())
					{
					case QDomNode::ElementNode:
						AppendElem (child.toElement ());
						break;
					case QDomNode::TextNode:
						AppendText (child.toText ());
						break;
					case QDomNode::AttributeNode:
						break;
					default:
						qWarning () << "unhandled node type"
								<< child.nodeType ();
						break;
					}
				}
			}

			QTextBlockFormat GetElemBlockFormat (const QDomElement& elem)
			{
				const auto& blockCfg = Config_.GetBlockFormat (elem.tagName (), elem.attribute ("class"_qs));

				QTextBlockFormat blockFmt;
				blockFmt.setAlignment (blockCfg.Align_);
				blockFmt.setLeftMargin (blockCfg.Margins_.left ());
				blockFmt.setTopMargin (blockCfg.Margins_.top ());
				blockFmt.setRightMargin (blockCfg.Margins_.right ());
				blockFmt.setBottomMargin (blockCfg.Margins_.bottom ());
				blockFmt.setTextIndent (blockCfg.Indent_);
				blockFmt.setHeadingLevel (blockCfg.HeadingLevel_);

				return blockFmt;
			}

			std::optional<QTextCharFormat> GetElemCharFormat (const QDomElement& elem)
			{
				if (const auto& charCfg = Config_.GetCharFormat (elem.tagName (), elem.attribute ("class"_qs)))
				{
					QTextCharFormat fmt;

					const auto set = [&fmt] (auto setter, const auto& maybeVal)
					{
						if (maybeVal)
							std::invoke (setter, fmt, *maybeVal);
					};

					set (&QTextCharFormat::setFontPointSize, charCfg->PointSize_);
					return fmt;
				}

				return {};
			}
		};
	}

	void Html2Doc (QTextDocument& doc, const QDomElement& body)
	{
		Converter { doc } (body);
	}
}
