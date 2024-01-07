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
#include "linksbuilder.h"
#include "stackkeeper.h"
#include "textdocumentformatconfig.h"
#include "tocbuilder.h"

namespace LC::Monocle
{
	namespace
	{
		bool IsHierBlockElem (QStringView tagName)
		{
			static const QSet names
			{
				u"div"_qsv,
				u"blockquote"_qsv,
			};
			return names.contains (tagName);
		}

		bool IsNonHierBlockElem (QStringView tagName)
		{
			static const QSet names
			{
				u"p"_qsv,
				u"h1"_qsv,
				u"h2"_qsv,
				u"h3"_qsv,
				u"h4"_qsv,
				u"h5"_qsv,
				u"h6"_qsv,
			};
			return names.contains (tagName);
		}

		class Converter
		{
			const TextDocumentFormatConfig& Config_ = TextDocumentFormatConfig::Instance ();

			QTextDocument& Doc_;
			QTextFrame& BodyFrame_;
			QTextCursor Cursor_;

			CustomStyler_f CustomStyler_;

			QTextCharFormat CharFormat_;

			TocBuilder TocBuilder_;
			LinksBuilder LinksBuilder_;

			StylingContext StylingCtx_;
		public:
			explicit Converter (QTextDocument& doc, const CustomStyler_f& styler, IDocument& monocleDoc)
			: Doc_ { doc }
			, BodyFrame_ { *QTextCursor { &doc }.insertFrame (Config_.GetBodyFrameFormat ()) }
			, Cursor_ { &BodyFrame_ }
			, CustomStyler_ { styler }
			, TocBuilder_ { Cursor_, monocleDoc }
			, LinksBuilder_ { Cursor_ }
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

			TOCEntryLevel_t GetTOC () const
			{
				return TocBuilder_.GetTOC ();
			}

			QVector<InternalLink> GetInternalLinks () const
			{
				return LinksBuilder_.GetLinks ();
			}
		private:
			void AppendElem (const QDomElement& elem)
			{
				StackKeeper stylingCtxKeeper { StylingCtx_ };
				const auto& klass = elem.attribute ("class"_qs);
				stylingCtxKeeper.Set ({ elem.tagName (), QStringView { klass }.split (' ') });

				StackKeeper charKeeper { CharFormat_ };
				if (auto maybeCharFmt = GetCharFormat ())
					charKeeper.Set (*std::move (maybeCharFmt));

				StackKeeper cursorKeeper { Cursor_ };

				if (IsHierBlockElem (elem.tagName ()))
				{
					const auto curFrame = Cursor_.currentFrame ();
					const auto [frameFmt, blockFmt] = GetFrameBlockFormat ();
					Cursor_.insertFrame (frameFmt);
					Cursor_.setBlockFormat (blockFmt);
					Cursor_.setBlockCharFormat (CharFormat_);

					cursorKeeper.Save (curFrame->lastCursorPosition ());
				}
				else if (IsNonHierBlockElem (elem.tagName ()))
				{
					const auto& blockFmt = GetBlockFormat ();
					if (!Cursor_.block ().text ().isEmpty ())
						Cursor_.insertBlock (blockFmt, CharFormat_);
					else
					{
						Cursor_.setBlockFormat (blockFmt);
						Cursor_.setBlockCharFormat (CharFormat_);
					}
				}

				const auto tocGuard = TocBuilder_.HandleElem (elem);
				const auto linkGuard = LinksBuilder_.HandleElem (elem);

				HandleElem (elem);
				HandleChildren (elem);
			}

			void AppendText (const QDomText& textNode)
			{
				auto text = textNode.data ();
				if (text.isEmpty ())
					return;

				if (auto simplified = std::move (text).simplified ();
					!simplified.isEmpty ())
					Cursor_.insertText (simplified, CharFormat_);
				else
					Cursor_.insertText (" "_qs, CharFormat_);
			}

			void HandleElem (const QDomElement& elem)
			{
				if (elem.tagName () == "br"_ql)
					Cursor_.insertText (QString { '\n' });

				if (elem.tagName () == "img"_ql)
					Cursor_.insertImage (elem.attribute ("src"_qs));
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

			template<typename T>
			void SetBlockConfig (T& marginsFmt, QTextBlockFormat& blockFmt, const BlockFormat& blockCfg)
			{
				const auto set = [] (auto& fmt, auto setter, const auto& maybeVal)
				{
					if (maybeVal)
						(fmt.*setter) (*maybeVal);
				};
				set (marginsFmt, &T::setLeftMargin, blockCfg.MarginLeft_);
				set (marginsFmt, &T::setTopMargin, blockCfg.MarginTop_);
				set (marginsFmt, &T::setRightMargin, blockCfg.MarginRight_);
				set (marginsFmt, &T::setBottomMargin, blockCfg.MarginBottom_);
				set (blockFmt, &QTextBlockFormat::setAlignment, blockCfg.Align_);
				set (blockFmt, &QTextBlockFormat::setTextIndent, blockCfg.Indent_);
				set (blockFmt, &QTextBlockFormat::setHeadingLevel, blockCfg.HeadingLevel_);
				set (blockFmt, &QTextBlockFormat::setForeground, blockCfg.Foreground_);
				set (blockFmt, &QTextBlockFormat::setBackground, blockCfg.Background_);
			}

			std::pair<QTextFrameFormat, QTextBlockFormat> GetFrameBlockFormat ()
			{
				auto blockCfg = Config_.GetBlockFormat (StylingCtx_);
				if (CustomStyler_)
					blockCfg += CustomStyler_ (StylingCtx_).first;

				QTextFrameFormat frameFmt;
				QTextBlockFormat blockFmt;
				SetBlockConfig (frameFmt, blockFmt, blockCfg);
				return { frameFmt, blockFmt };
			}

			QTextBlockFormat GetBlockFormat ()
			{
				auto blockCfg = Config_.GetBlockFormat (StylingCtx_);
				if (CustomStyler_)
					blockCfg += CustomStyler_ (StylingCtx_).first;

				QTextBlockFormat blockFmt;
				SetBlockConfig (blockFmt, blockFmt, blockCfg);
				return blockFmt;
			}

			std::optional<QTextCharFormat> GetCharFormat ()
			{
				auto charCfg = Config_.GetCharFormat (StylingCtx_);
				const auto& custom = CustomStyler_ ? CustomStyler_ (StylingCtx_).second : CharFormat {};
				if (!charCfg && custom.IsEmpty ())
					return {};
				else if (!charCfg)
					charCfg.emplace (CharFormat {});

				*charCfg += custom;

				auto fmt = CharFormat_;
				const auto set = [&fmt] (auto setter, const auto& maybeVal)
				{
					if (maybeVal)
						(fmt.*setter) (*maybeVal);
				};
				set (&QTextCharFormat::setFontPointSize, charCfg->PointSize_);
				set (&QTextCharFormat::setFontWeight, charCfg->IsBold_);
				set (&QTextCharFormat::setFontItalic, charCfg->IsItalic_);
				set (&QTextCharFormat::setFontUnderline, charCfg->IsUnderline_);
				set (&QTextCharFormat::setFontStrikeOut, charCfg->IsStrikeThrough_);
				set (&QTextCharFormat::setVerticalAlignment, charCfg->VerticalAlignment_);
				set (&QTextCharFormat::setForeground, charCfg->Foreground_);
				set (&QTextCharFormat::setBackground, charCfg->Background_);
				return fmt;
			}
		};
	}

	DocStructure Html2Doc (QTextDocument& doc,
			const QDomElement& body,
			const CustomStyler_f& styler,
			IDocument& monocleDoc)
	{
		Converter conv { doc, styler, monocleDoc };
		conv (body);
		return { .TOC_ = conv.GetTOC (), .InternalLinks_ = conv.GetInternalLinks () };
	}
}
