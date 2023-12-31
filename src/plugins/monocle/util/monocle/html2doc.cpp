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

			QTextBlockFormat BlockFormat_;
			QTextCharFormat CharFormat_;
		public:
			explicit Converter (QTextDocument& doc)
			: Doc_ { doc }
			, BodyFrame_ { *QTextCursor { &doc }.insertFrame (Config_.GetBodyFrameFormat ()) }
			, Cursor_ { &BodyFrame_ }
			{
				auto rootFmt = Doc_.rootFrame ()->frameFormat ();
				rootFmt.setBackground (Config_.GetPalette ().Background_);
				rootFmt.setForeground (Config_.GetPalette ().Foreground_);
				Doc_.rootFrame ()->setFrameFormat (rootFmt);
			}

			void operator() (const QDomElement& body)
			{
				AppendElem (body);
			}
		private:
			void AppendElem (const QDomElement& elem)
			{
				FormatKeeper blockKeeper { BlockFormat_ };
				blockKeeper.SetFormat (GetElemBlockFormat (elem));

				FormatKeeper charKeeper { CharFormat_ };
				if (auto maybeCharFmt = GetElemCharFormat (elem))
					charKeeper.SetFormat (*std::move (maybeCharFmt));

				if (IsBlockElem (elem.tagName ()))
				{
					if (!Cursor_.block ().text ().isEmpty ())
						Cursor_.insertBlock (BlockFormat_, CharFormat_);
					else
					{
						Cursor_.setBlockFormat (BlockFormat_);
						Cursor_.setCharFormat (CharFormat_);
					}
				}

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

				auto fmt = BlockFormat_;
				const auto set = [&fmt] (auto setter, const auto& maybeVal)
				{
					if (maybeVal)
						std::invoke (setter, fmt, *maybeVal);
				};
				set (&QTextBlockFormat::setAlignment, blockCfg.Align_);
				set (&QTextBlockFormat::setAlignment, blockCfg.Align_);
				set (&QTextBlockFormat::setLeftMargin, blockCfg.MarginLeft_);
				set (&QTextBlockFormat::setTopMargin, blockCfg.MarginTop_);
				set (&QTextBlockFormat::setRightMargin, blockCfg.MarginRight_);
				set (&QTextBlockFormat::setBottomMargin, blockCfg.MarginBottom_);
				set (&QTextBlockFormat::setTextIndent, blockCfg.Indent_);
				set (&QTextBlockFormat::setHeadingLevel, blockCfg.HeadingLevel_);
				return fmt;
			}

			std::optional<QTextCharFormat> GetElemCharFormat (const QDomElement& elem)
			{
				const auto& charCfg = Config_.GetCharFormat (elem.tagName (), elem.attribute ("class"_qs));
				if (!charCfg)
					return {};

				auto fmt = CharFormat_;
				const auto set = [&fmt] (auto setter, const auto& maybeVal)
				{
					if (maybeVal)
						std::invoke (setter, fmt, *maybeVal);
				};
				set (&QTextCharFormat::setFontPointSize, charCfg->PointSize_);
				set ([] (auto& fmt, bool bold) { fmt.setFontWeight (bold ? QFont::Bold : QFont::Normal); }, charCfg->IsBold_);
				set (&QTextCharFormat::setFontItalic, charCfg->IsItalic_);
				set (&QTextCharFormat::setFontUnderline, charCfg->IsUnderline_);
				set (&QTextCharFormat::setFontStrikeOut, charCfg->IsStrikeThrough_);
				return fmt;
			}
		};
	}

	void Html2Doc (QTextDocument& doc, const QDomElement& body)
	{
		Converter { doc } (body);
	}
}
