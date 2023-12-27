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
		doc.setDefaultFont (XSM_->property ("DefaultFont").value<QFont> ());

		const QSize pageSize { XSM_->property ("PageWidth").toInt (), XSM_->property ("PageHeight").toInt () };
		doc.setPageSize (pageSize);
		doc.setUndoRedoEnabled (false);

		const auto& palette = GetPalette ();

		const auto rootFrame = doc.rootFrame ();
		auto frameFmt = rootFrame->frameFormat ();
		const auto margin = [this] (const QByteArray& orient) { return XSM_->property (orient + "Margin").toInt (); };
		frameFmt.setLeftMargin (margin ("Left"));
		frameFmt.setRightMargin (margin ("Right"));
		frameFmt.setTopMargin (margin ("Top"));
		frameFmt.setBottomMargin (margin ("Bottom"));
		frameFmt.setBackground (palette.Background_);
		rootFrame->setFrameFormat (frameFmt);
	}

	void TextDocumentFormatConfig::SetXSM (Util::BaseSettingsManager& xsm)
	{
		XSM_ = &xsm;

		UpdatePalette ();
	}

	void TextDocumentFormatConfig::UpdatePalette ()
	{
		Palette_ =
		{
			.Background_ = qGuiApp->palette ().color (QPalette::Base),
			.Link_ = qGuiApp->palette ().color (QPalette::Link),
		};
	}
}
