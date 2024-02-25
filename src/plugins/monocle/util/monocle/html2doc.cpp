/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "html2doc.h"
#include <QDomElement>
#include <QRegularExpression>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFrame>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include "imghandler.h"
#include "formatutils.h"
#include "linksbuilder.h"
#include "resourcedtextdocument.h"
#include "stackkeeper.h"
#include "tablehandler.h"
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

		QString NormalizeSpaces (QString&& str)
		{
			static const QRegularExpression spaces { "\\s+"_qs };
			return std::move (str).replace (spaces, " "_ql);
		}

		class StylingContextKeeper
		{
			const QTextCharFormat& CharFormat_;

			QString Tag_;
			QString Classes_;

			StylingContextElement CurrElem_;
			QVector<StylingContextElement> Parents_;
		public:
			StylingContextKeeper (const QTextCharFormat& cfmt)
			: CharFormat_ { cfmt }
			{
			}

			auto HandleElem (const QDomElement& elem)
			{
				auto prevTag = Tag_;
				auto prevClasses = Classes_;

				Tag_ = elem.tagName ();
				Classes_ = elem.attribute ("class"_qs);

				Parents_.push_back (CurrElem_);
				CurrElem_ = StylingContextElement { Tag_, QStringView { Classes_ }.split (' ', Qt::SkipEmptyParts) };

				return Util::MakeScopeGuard ([this, prevTag, prevClasses]
						{
							CurrElem_ = Parents_.takeLast ();
							Tag_ = prevTag;
							Classes_ = prevClasses;
						});
			}

			StylingContext GetContext () const
			{
				return { CurrElem_, Parents_, CharFormat_ };
			}
		};

		class Converter
		{
			const TextDocumentFormatConfig& Config_ = TextDocumentFormatConfig::Instance ();

			const CustomStyler_f& CustomStyler_;

			ResourcedTextDocument& Doc_;
			QTextFrame& BodyFrame_;
			QTextCursor Cursor_;

			QTextCharFormat CharFormat_;

			TocBuilder TocBuilder_;
			LinksBuilder LinksBuilder_;
			ImgHandler ImgHandler_;

			StylingContextKeeper StylingCtxKeeper_ { CharFormat_ };

			std::list<TableHandler> TableHandlers_;
		public:
			explicit Converter (ResourcedTextDocument& doc,
					const TOCEntryID& tocStructure,
					const CustomStyler_f& styler,
					const LazyImages_t& images)
			: CustomStyler_ { styler }
			, Doc_ { doc }
			, BodyFrame_ { *QTextCursor { &doc }.insertFrame (Config_.GetBodyFrameFormat ()) }
			, Cursor_ { &BodyFrame_ }
			, TocBuilder_ { tocStructure, Cursor_ }
			, LinksBuilder_ { Cursor_ }
			, ImgHandler_ { Cursor_, styler, images }
			{
				auto rootFmt = Doc_.rootFrame ()->frameFormat ();
				rootFmt.setBackground (Config_.GetPalette ().Background_);
				rootFmt.setForeground (Config_.GetPalette ().Foreground_);
				Doc_.rootFrame ()->setFrameFormat (rootFmt);
			}

			void operator() (const QDomElement& body)
			{
				AppendElem (body);
				Doc_.SetMaxImageSizes (ImgHandler_.GetMaxSizes ());
			}

			TOCEntryLevelT<Span> GetTOC () const
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
				const auto ctxGuard = StylingCtxKeeper_.HandleElem (elem);

				StackKeeper charKeeper { CharFormat_ };
				if (auto maybeCharFmt = GetCharFormat ())
					charKeeper.Set (*std::move (maybeCharFmt));

				const auto formatGuard = HandleFormatContext (elem);

				if (!TableHandlers_.empty ())
					TableHandlers_.back ().HandleElem (elem);

				TocBuilder_.HandleElem (elem);
				const auto linkGuard = LinksBuilder_.HandleElem (elem);

				HandleElem (elem);
				HandleChildren (elem);
			}

			Util::DefaultScopeGuard HandleFormatContext (const QDomElement& elem)
			{
				if (elem.tagName () == "table"_ql)
					return CreateTable (elem);
				if (IsHierBlockElem (elem.tagName ()))
					return CreateFrame ();
				if (IsNonHierBlockElem (elem.tagName ()))
					return CreateBlock ();

				return {};
			}

			Util::DefaultScopeGuard CreateTable (const QDomElement& tableElem)
			{
				TableHandlers_.emplace_back (tableElem, Cursor_, GetFrameBlockFormat ().first);
				return Util::MakeScopeGuard ([this] { TableHandlers_.pop_back (); });
			}

			Util::DefaultScopeGuard CreateFrame ()
			{
				const auto curFrame = Cursor_.currentFrame ();
				const auto [frameFmt, blockFmt] = GetFrameBlockFormat ();
				if (frameFmt != curFrame->frameFormat ())
				{
					Cursor_.insertFrame (frameFmt);
					Cursor_.setBlockFormat (blockFmt);
					Cursor_.setBlockCharFormat (CharFormat_);
				}
				else
					Cursor_.insertBlock (blockFmt, CharFormat_);

				return Util::MakeScopeGuard ([this, curFrame] { Cursor_ = curFrame->lastCursorPosition (); });
			}

			Util::DefaultScopeGuard CreateBlock ()
			{
				const auto& blockFmt = GetBlockFormat ();
				if (!Cursor_.block ().text ().isEmpty ())
					Cursor_.insertBlock (blockFmt, CharFormat_);
				else
				{
					Cursor_.setBlockFormat (blockFmt);
					Cursor_.setBlockCharFormat (CharFormat_);
				}

				return {};
			}

			void HandleElem (const QDomElement& elem)
			{
				if (elem.tagName () == "br"_ql)
					Cursor_.insertText (QString { '\n' });

				if (elem.tagName () == "img"_ql)
					ImgHandler_.HandleImg (elem, StylingCtxKeeper_.GetContext ());
			}

			void HandleChildren (const QDomElement& elem)
			{
				const auto& children = elem.childNodes ();
				for (int i = 0, size = children.size (); i < size; ++i)
				{
					const auto& child = children.at (i);
					switch (child.nodeType ())
					{
					case QDomNode::ElementNode:
						AppendElem (child.toElement ());
						break;
					case QDomNode::TextNode:
						if (auto text = child.toText ().data (); !text.isEmpty ())
							Cursor_.insertText (NormalizeSpaces (std::move (text)), CharFormat_);
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

			std::pair<QTextFrameFormat, QTextBlockFormat> GetFrameBlockFormat () const
			{
				auto blockCfg = Config_.GetBlockFormat (StylingCtxKeeper_.GetContext ());
				if (CustomStyler_)
					blockCfg += CustomStyler_ (StylingCtxKeeper_.GetContext ()).Block_;

				QTextFrameFormat frameFmt;
				QTextBlockFormat blockFmt;
				SetBlockConfig (frameFmt, blockFmt, blockCfg);
				return { frameFmt, blockFmt };
			}

			QTextBlockFormat GetBlockFormat () const
			{
				auto blockCfg = Config_.GetBlockFormat (StylingCtxKeeper_.GetContext ());
				if (CustomStyler_)
					blockCfg += CustomStyler_ (StylingCtxKeeper_.GetContext ()).Block_;

				QTextBlockFormat blockFmt;
				SetBlockConfig (blockFmt, blockFmt, blockCfg);
				return blockFmt;
			}

			std::optional<QTextCharFormat> GetCharFormat () const
			{
				auto charCfg = Config_.GetCharFormat (StylingCtxKeeper_.GetContext ());
				const auto& custom = CustomStyler_ ? CustomStyler_ (StylingCtxKeeper_.GetContext ()).Char_ : CharFormat {};
				if (!charCfg && custom.IsEmpty ())
					return {};
				if (!charCfg)
					charCfg.emplace (CharFormat {});

				*charCfg += custom;

				auto fmt = CharFormat_;
				SetCharConfig (fmt, *charCfg);
				return fmt;
			}
		};
	}

	DocStructure Html2Doc (const HtmlConvContext& ctx)
	{
		Converter conv { ctx.Doc_, ctx.TocStructure_, ctx.Styler_, ctx.Images_ };
		conv (ctx.Body_);
		return { .TOC_ = conv.GetTOC (), .InternalLinks_ = conv.GetInternalLinks () };
	}
}
