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

		class Converter
		{
			const TextDocumentFormatConfig& Config_ = TextDocumentFormatConfig::Instance ();

			QTextDocument& Doc_;
			QTextFrame& BodyFrame_;
			QTextCursor Cursor_;
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
				HandleElem (elem);

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

			void AppendText (const QDomText& textNode)
			{
				Cursor_.insertText (textNode.data ());
			}

			void HandleElem (const QDomElement& elem)
			{
				if (IsBlockElem (elem.tagName ()))
				{
					SetupBlock (elem);
					return;
				}

				if (elem.tagName () == "br"_ql)
					Cursor_.insertText (QString { '\n' });
			}

			void SetupBlock (const QDomElement& elem)
			{
				const auto& [blockFmt, maybeCharFmt] = GetElemBlockFormat (elem);
				if (maybeCharFmt)
					Cursor_.insertBlock (blockFmt, *maybeCharFmt);
				else
					Cursor_.insertBlock (blockFmt, {});
			}

			std::pair<QTextBlockFormat, std::optional<QTextCharFormat>> GetElemBlockFormat (const QDomElement& elem)
			{
				QTextBlockFormat blockFmt;
				const auto& [blockCfg, maybeCharCfg] = Config_.GetBlockFormat (elem.tagName (), elem.attribute ("class"));
				blockFmt.setAlignment (blockCfg.Align_);
				blockFmt.setLeftMargin (blockCfg.Margins_.left ());
				blockFmt.setTopMargin (blockCfg.Margins_.top ());
				blockFmt.setRightMargin (blockCfg.Margins_.right ());
				blockFmt.setBottomMargin (blockCfg.Margins_.bottom ());
				blockFmt.setTextIndent (blockCfg.Indent_);
				blockFmt.setHeadingLevel (blockCfg.HeadingLevel_);

				std::optional<QTextCharFormat> maybeCharFmt;
				if (maybeCharCfg)
				{
					QTextCharFormat fmt;
					fmt.setFontPointSize (maybeCharCfg->PointSize_);
					maybeCharFmt = fmt;
				}

				return { blockFmt, maybeCharFmt };
			}
		};
	}

	void Html2Doc (QTextDocument& doc, const QDomElement& body)
	{
		Converter { doc } (body);
	}
}
