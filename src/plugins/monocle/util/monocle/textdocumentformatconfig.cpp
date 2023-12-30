/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "textdocumentformatconfig.h"
#include <QGuiApplication>
#include <QPalette>
#include <QTextDocument>
#include <QTextFrame>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <xmlsettingsdialog/basesettingsmanager.h>

namespace LC::Monocle
{
	TextDocumentFormatConfig& TextDocumentFormatConfig::Instance ()
	{
		static TextDocumentFormatConfig tdfc;
		return tdfc;
	}

	const TextDocumentPalette& TextDocumentFormatConfig::GetPalette () const
	{
		return Palette_;
	}

	void TextDocumentFormatConfig::FormatDocument (QTextDocument& doc) const
	{
		const QSize pageSize { XSM_->property ("PageWidth").toInt (), XSM_->property ("PageHeight").toInt () };
		doc.setPageSize (pageSize);
		doc.setUndoRedoEnabled (false);

		doc.setDefaultFont (XSM_->property ("DefaultFont").value<QFont> ());

		const auto rootFrame = doc.rootFrame ();
		auto frameFmt = rootFrame->frameFormat ();
		const auto margin = [this] (const QByteArray& orient) { return XSM_->property (orient + "Margin").toInt (); };
		frameFmt.setLeftMargin (margin ("Left"));
		frameFmt.setRightMargin (margin ("Right"));
		frameFmt.setTopMargin (margin ("Top"));
		frameFmt.setBottomMargin (margin ("Bottom"));
		frameFmt.setWidth (pageSize.width ());
		rootFrame->setFrameFormat (frameFmt);
	}

	QString TextDocumentFormatConfig::GetStyleSheet () const
	{
		const auto& palette = GetPalette ();

		// TODO make much of this configurable
		return R"(
			body {
				background-color: ${bgcolor};
			}

			p {
				margin-top: 0px;
				margin-bottom: 0px;
				text-indent: 16px;
			}

			.break-after {
				page-break-after: always;
			}

			.poem {
				text-align: left;
				font-style: italic;
				margin-left: 4em;
			}

			.style-emphasis {
				text-decoration: underline;
			}

			.stanza {
				margin-top: 0.5em;
			}

			.epigraph {
				text-align: right;
			}
		)"_qs
			.replace ("${bgcolor}"_ql, palette.Background_.name ())
			;
	}

	NonStyleSheetStyles TextDocumentFormatConfig::GetNonStyleSheetStyles () const
	{
		// TODO make configurable
		return
		{
			.AlignP_ = Qt::AlignJustify,
			.AlignH_ = { Qt::AlignHCenter, Qt::AlignHCenter, Qt::AlignLeft, Qt::AlignLeft, Qt::AlignLeft, Qt::AlignLeft },
		};
	}

	void TextDocumentFormatConfig::SetXSM (Util::BaseSettingsManager& xsm)
	{
		XSM_ = &xsm;

		UpdatePalette ();
	}

	void TextDocumentFormatConfig::UpdatePalette ()
	{
		// TODO make configurable
		Palette_ =
		{
			.Background_ = qGuiApp->palette ().color (QPalette::Base),
			.Link_ = qGuiApp->palette ().color (QPalette::Link),
		};
	}
}
