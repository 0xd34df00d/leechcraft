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
#include <QUrl>
#include <QImage>
#include <QVariant>
#include <QStringList>
#include <QtDebug>
#include <util/sll/util.h>
#include <util/sll/either.h>
#include <util/sll/qtutil.h>
#include <util/sll/domchildrenrange.h>
#include "toclink.h"

namespace LC
{
namespace Monocle
{
namespace FXB
{
	class CursorCacher
	{
		QTextCursor * const Cursor_;

		QString Text_;

		QTextBlockFormat LastBlockFormat_ = Cursor_->blockFormat ();
		QTextCharFormat LastCharFormat_ = Cursor_->charFormat ();
	public:
		CursorCacher (QTextCursor *cursor);
		~CursorCacher ();

		const QTextBlockFormat& blockFormat () const;
		const QTextCharFormat& charFormat () const;

		void insertBlock (const QTextBlockFormat&);
		void insertText (const QString&);

		void setCharFormat (const QTextCharFormat&);

		void Flush ();
	};

	CursorCacher::CursorCacher (QTextCursor *cursor)
	: Cursor_ (cursor)
	{
	}

	CursorCacher::~CursorCacher ()
	{
		Flush ();
	}

	const QTextBlockFormat& CursorCacher::blockFormat () const
	{
		return LastBlockFormat_;
	}

	const QTextCharFormat& CursorCacher::charFormat () const
	{
		return LastCharFormat_;
	}

	void CursorCacher::insertBlock (const QTextBlockFormat& fmt)
	{
		if (fmt == LastBlockFormat_)
		{
			Text_ += "\n";
			return;
		}

		Flush ();

		Cursor_->insertBlock (fmt);
		LastBlockFormat_ = fmt;
	}

	void CursorCacher::insertText (const QString& text)
	{
		Text_ += text;
	}

	void CursorCacher::setCharFormat (const QTextCharFormat& fmt)
	{
		if (LastCharFormat_ == fmt)
			return;

		Flush ();

		Cursor_->setCharFormat (fmt);
		LastCharFormat_ = fmt;
	}

	void CursorCacher::Flush ()
	{
		if (Text_.isEmpty ())
			return;

		Cursor_->insertText (Text_);
		Text_.clear ();
	}

	FB2Converter::FB2Converter (Document *doc, const QDomDocument& fb2, const Config& config)
	: ParentDoc_ { doc }
	, FB2_ { fb2 }
	, Result_ { std::make_shared<QTextDocument> () }
	, Cursor_ { std::make_unique<QTextCursor> (Result_.get ()) }
	, CursorCacher_ { std::make_unique<CursorCacher> (Cursor_.get ()) }
	, Config_ { config }
	{
		Result_->setDefaultFont (config.DefaultFont_);
		Result_->setPageSize (config.PageSize_);
		Result_->setUndoRedoEnabled (false);

		const auto& docElem = FB2_.documentElement ();
		if (docElem.tagName () != "FictionBook")
		{
			Error_ = NotAnFBDocument {};
			return;
		}

		const auto rootFrame = Result_->rootFrame ();

		auto frameFmt = rootFrame->frameFormat ();

		frameFmt.setLeftMargin (config.Margins_.left ());
		frameFmt.setRightMargin (config.Margins_.right ());
		frameFmt.setTopMargin (config.Margins_.top ());
		frameFmt.setBottomMargin (config.Margins_.bottom ());

		frameFmt.setBackground (config.BackgroundColor_);
		rootFrame->setFrameFormat (frameFmt);

		Handlers_ ["section"] = [this] (const QDomElement& p) { HandleSection (p); };
		Handlers_ ["title"] = [this] (const QDomElement& p) { HandleTitle (p); };
		Handlers_ ["subtitle"] = [this] (const QDomElement& p) { HandleTitle (p, 1); };
		Handlers_ ["epigraph"] = [this] (const QDomElement& p) { HandleEpigraph (p); };
		Handlers_ ["image"] = [this] (const QDomElement& p) { HandleImage (p); };

		Handlers_ ["p"] = [this] (const QDomElement& p) { HandlePara (p); };
		Handlers_ ["poem"] = [this] (const QDomElement& p) { HandlePoem (p); };
		Handlers_ ["empty-line"] = [this] (const QDomElement& p) { HandleEmptyLine (p); };
		Handlers_ ["stanza"] = [this] (const QDomElement& p) { HandleStanza (p); };
		Handlers_ ["v"] = [this] (const QDomElement& p)
		{
			auto blockFmt = CursorCacher_->blockFormat ();
			blockFmt.setTextIndent (50);

			CursorCacher_->insertBlock (blockFmt);

			HandleMangleCharFormat (p,
					[] (QTextCharFormat& fmt) { fmt.setFontItalic (true); },
					[this] (const QDomElement& p) { HandleParaWONL (p); });
		};

		Handlers_ ["emphasis"] = [this] (const QDomElement& p)
		{
			HandleMangleCharFormat (p,
					[] (QTextCharFormat& fmt) { fmt.setFontItalic (true); },
					[this] (const QDomElement& p) { HandleParaWONL (p); });
		};
		Handlers_ ["strong"] = [this] (const QDomElement& p)
		{
			HandleMangleCharFormat (p,
					[] (QTextCharFormat& fmt) { fmt.setFontWeight (QFont::Bold); },
					[this] (const QDomElement& p) { HandleParaWONL (p); });
		};
		Handlers_ ["strikethrough"] = [this] (const QDomElement& p)
		{
			HandleMangleCharFormat (p,
					[] (QTextCharFormat& fmt) { fmt.setFontStrikeOut (true); },
					[this] (const QDomElement& p) { HandleParaWONL (p); });
		};

		Handlers_ ["annotation"] = [this] (const QDomElement& p)
		{
			HandleMangleBlockFormat (p,
					[] (QTextBlockFormat& fmt)
					{
						fmt.setAlignment (Qt::AlignRight);
						fmt.setLeftMargin (60);
					},
					[this] (const QDomElement& p)
					{
						HandleMangleCharFormat (p,
								[] (QTextCharFormat& fmt) { fmt.setFontItalic (true); },
								[this] (const QDomElement& p) { HandleChildren (p); });
					});
		};
		Handlers_ ["style"] = [this] (const QDomElement& p) { HandleParaWONL (p); };
		Handlers_ ["coverpage"] = [this] (const QDomElement& p) { HandleChildren (p); };
		Handlers_ ["a"] = [this] (const QDomElement& p) { HandleLink (p); };

		TOCEntry entry =
		{
			ILink_ptr (),
			"root",
			TOCEntryLevel_t ()
		};
		CurrentTOCStack_.push (&entry);

		for (const auto& elem : Util::DomChildren (docElem, {}))
		{
			const auto& tagName = elem.tagName ();
			if (tagName == "description")
				HandleDescription (elem);
			else if (tagName == "body")
				HandleBody (elem);
		}

		TOC_ = entry.ChildLevel_;

		CursorCacher_->Flush ();

		for (const auto& [tag, count] : Util::Stlize (UnhandledTags_))
			qWarning () << "unknown tag" << tag << "occurred" << count << "times";
	}

	FB2Converter::~FB2Converter () = default;

	FB2Converter::ConversionResult_t FB2Converter::GetResult () const
	{
		if (Error_)
			return ConversionResult_t::Left (*Error_);
		else
			return ConversionResult_t::Right ({
					Result_,
					DocInfo_,
					TOC_,
					GetLinks ()
				});
	}

	QList<TextDocumentAdapter::InternalLink> FB2Converter::GetLinks () const
	{
		QHash<QString, QPair<int, int>> targetsHash;
		for (const auto& target : LinkTargets_)
		{
			if (targetsHash.contains (target.Anchor_))
				qWarning () << Q_FUNC_INFO
						<< target.Anchor_
						<< "is already present";

			targetsHash [target.Anchor_] = target.Span_;
		}

		QList<TextDocumentAdapter::InternalLink> result;
		for (const auto& source : LinkSources_)
		{
			if (!targetsHash.contains (source.Anchor_))
			{
				qWarning () << Q_FUNC_INFO
						<< "unknown target"
						<< source.Anchor_;
				continue;
			}

			result.push_back ({ source.Span_, targetsHash [source.Anchor_] });
		}
		return result;
	}

	QDomElement FB2Converter::FindBinary (const QString& refId) const
	{
		const auto& binaries = FB2_.elementsByTagName ("binary");
		for (int i = 0; i < binaries.size (); ++i)
		{
			const auto& elem = binaries.at (i).toElement ();
			if (elem.attribute ("id") == refId)
				return elem;
		}

		return {};
	}

	void FB2Converter::HandleDescription (const QDomElement& elem)
	{
		QStringList handledChildren;
		auto getChildValues = [&elem, &handledChildren] (const QString& nodeName) -> QStringList
		{
			handledChildren << nodeName;

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
		DocInfo_.Keywords_ = getChildValues ("keywords").value (0).split (' ', Qt::SkipEmptyParts);

		const auto& dateElem = elem.elementsByTagName ("date").at (0).toElement ();
		DocInfo_.Date_.setDate (QDate::fromString (dateElem.attribute ("value"), Qt::ISODate));

		DocInfo_.Author_ += getChildValues ("first-name").value (0) + " ";
		DocInfo_.Author_ += getChildValues ("last-name").value (0) + " ";
		const auto& email = getChildValues ("email").value (0);
		if (!email.isEmpty ())
			DocInfo_.Author_ += "<" + email + "> ";
		DocInfo_.Author_ += getChildValues ("nickname").value (0);

		DocInfo_.Author_ = DocInfo_.Author_.trimmed ().simplified ();

		FillPreamble ();

		for (const auto& childElem : Util::DomChildren (elem.firstChildElement ("title-info"), {}))
			if (!handledChildren.contains (childElem.tagName ()))
				Handle (childElem);
	}

	void FB2Converter::HandleBody (const QDomElement& bodyElem)
	{
		HandleChildren (bodyElem);
	}

	namespace
	{
		class CursorSpanKeeper
		{
			CursorCacher& Cacher_;
			const QTextCursor& Cursor_;

			int StartPosition_;
		public:
			CursorSpanKeeper (CursorCacher& cacher, const QTextCursor& cursor)
			: Cacher_ { cacher }
			, Cursor_ { cursor }
			{
				Cacher_.Flush ();
				StartPosition_ = Cursor_.position ();
			}

			QPair<int, int> GetSpan () const
			{
				Cacher_.Flush ();
				return { StartPosition_, Cursor_.position () };
			}
		};

		class LinkCtxHandler
		{
			const QString Anchor_;
			QList<FB2Converter::LinkCtx>& LinkCtxList_;

			std::optional<CursorSpanKeeper> SpanKeeper_;
		public:
			LinkCtxHandler (const QString& anchor, QList<FB2Converter::LinkCtx>& list,
					CursorCacher& cacher, const QTextCursor& cursor)
			: Anchor_ { anchor }
			, LinkCtxList_ { list }
			{
				if (!anchor.isEmpty ())
					SpanKeeper_.emplace (cacher, cursor);
			}

			~LinkCtxHandler ()
			{
				if (!SpanKeeper_)
					return;

				LinkCtxList_ << FB2Converter::LinkCtx { Anchor_, SpanKeeper_->GetSpan () };
			}
		};
	}

	void FB2Converter::HandleSection (const QDomElement& tagElem)
	{
		LinkCtxHandler linkHandler { tagElem.attribute ("id"), LinkTargets_, *CursorCacher_, *Cursor_ };

		CurrentTOCStack_.top ()->ChildLevel_.append (TOCEntry ());
		CurrentTOCStack_.push (&CurrentTOCStack_.top ()->ChildLevel_.last ());

		HandleChildren (tagElem);

		CurrentTOCStack_.pop ();
	}

	namespace
	{
		std::optional<QString> GetTitleName (const QDomElement& tagElem)
		{
			for (const auto& child : Util::DomChildren (tagElem, {}))
				if (child.tagName () == "p")
					return child.text ();

			return {};
		}

		/** Returns 0 if there are no empty lines or there are elements
		 * other than empty lines, returns the count of empty lines
		 * otherwise.
		 */
		int GetEmptyLinesCount (const QDomElement& elem)
		{
			int emptyCount = 0;
			for (const auto& child : Util::DomChildren (elem, {}))
			{
				if (child.tagName () != "empty-line")
					return 0;
				++emptyCount;
			}
			return emptyCount;
		}
	}

	void FB2Converter::HandleTitle (const QDomElement& tagElem, int level)
	{
		if (CurrentTOCStack_.top ()->Name_.isEmpty ())
			if (const auto name = GetTitleName (tagElem))
				*CurrentTOCStack_.top () = TOCEntry
						{
							std::make_shared<TOCLink> (ParentDoc_, Result_->pageCount () - 1),
							*name,
							{}
						};

		if (const auto emptyCount = GetEmptyLinesCount (tagElem))
		{
			CursorCacher_->insertText ("\n\n");
			for (int i = 0; i < emptyCount; ++i)
				CursorCacher_->insertText ("\n");
			return;
		}

		const auto currentSectionLevel = CurrentTOCStack_.size () - 1;
		HandleMangleBlockFormat (tagElem,
				[] (QTextBlockFormat&) {},
				[=, this] (const QDomElement& e)
				{
					HandleMangleCharFormat (e,
							[=] (QTextCharFormat& fmt)
							{
								const auto newSize = 18 - 2 * level - currentSectionLevel;
								fmt.setFontPointSize (newSize);
								fmt.setFontWeight (currentSectionLevel <= 1 ? QFont::Bold : QFont::DemiBold);
							},
							[this] (const QDomElement& e) { HandleParaWONL (e); });
				});
	}

	void FB2Converter::HandleEpigraph (const QDomElement& tagElem)
	{
		HandleChildren (tagElem);
	}

	void FB2Converter::HandleImage (const QDomElement& imageElem)
	{
		const auto& refId = imageElem.attribute ("href").mid (1);
		const auto& binary = FindBinary (refId);
		const auto& imageData = QByteArray::fromBase64 (binary.text ().toLatin1 ());
		const auto& image = QImage::fromData (imageData);

		Result_->addResource (QTextDocument::ImageResource,
				{ "image://" + refId },
				QVariant::fromValue (image));

		CursorCacher_->Flush ();
		Cursor_->insertHtml (QString ("<img src='image://%1'/>").arg (refId));
	}

	void FB2Converter::HandlePara (const QDomElement& tagElem)
	{
		auto fmt = CursorCacher_->blockFormat ();
		fmt.setTextIndent (20);
		fmt.setAlignment (Qt::AlignJustify);
		CursorCacher_->insertBlock (fmt);

		HandleParaWONL (tagElem);
	}

	void FB2Converter::HandleParaWONL (const QDomElement& tagElem)
	{
		auto child = tagElem.firstChild ();
		while (!child.isNull ())
		{
			const auto guard = Util::MakeScopeGuard ([&child] { child = child.nextSibling (); });

			if (child.isText ())
			{
				CursorCacher_->insertText (child.toText ().data ());
				continue;
			}

			if (!child.isElement ())
				continue;

			Handle (child.toElement ());
		}
	}

	void FB2Converter::HandlePoem (const QDomElement& tagElem)
	{
		HandleChildren (tagElem);
	}

	void FB2Converter::HandleStanza (const QDomElement& tagElem)
	{
		HandleChildren (tagElem);
	}

	void FB2Converter::HandleEmptyLine (const QDomElement&)
	{
		CursorCacher_->insertText ("\n\n");
	}

	void FB2Converter::HandleLink (const QDomElement& tagElem)
	{
		auto target = tagElem.attribute ("href");
		if (target.size () > 1 && target [0] == '#')
			target = target.mid (1);
		LinkCtxHandler linkHandler { target, LinkSources_, *CursorCacher_, *Cursor_ };

		HandleMangleCharFormat (tagElem,
				[this] (QTextCharFormat& fmt)
				{
					fmt.setFontUnderline (true);
					fmt.setForeground (Config_.LinkColor_);
				},
				[this] (const QDomElement& p) { HandleParaWONL (p); });
	}

	void FB2Converter::HandleChildren (const QDomElement& tagElem)
	{
		for (const auto& elem : Util::DomChildren (tagElem, {}))
			Handle (elem);
	}

	void FB2Converter::Handle (const QDomElement& child)
	{
		const auto& tagName = child.tagName ();
		const auto defaultHandler = [this, &tagName] (const QDomElement&) { ++UnhandledTags_ [tagName]; };
		Handlers_.value (tagName, defaultHandler) (child);
	}

	void FB2Converter::HandleMangleBlockFormat (const QDomElement& tagElem,
			std::function<void (QTextBlockFormat&)> mangler, Handler_f next)
	{
		const auto origFmt = CursorCacher_->blockFormat ();

		auto mangledFmt = origFmt;
		mangler (mangledFmt);
		CursorCacher_->insertBlock (mangledFmt);

		next ({ tagElem });

		CursorCacher_->insertBlock (origFmt);
	}

	void FB2Converter::HandleMangleCharFormat (const QDomElement& tagElem,
			std::function<void (QTextCharFormat&)> mangler, Handler_f next)
	{
		const auto origFmt = CursorCacher_->charFormat ();

		auto mangledFmt = origFmt;
		mangler (mangledFmt);
		CursorCacher_->setCharFormat (mangledFmt);

		next ({ tagElem });

		CursorCacher_->setCharFormat (origFmt);
	}

	void FB2Converter::FillPreamble ()
	{
		CursorCacher_->Flush ();

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
}
}
}
