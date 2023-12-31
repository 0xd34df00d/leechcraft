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
	}

	QTextFrameFormat TextDocumentFormatConfig::GetBodyFrameFormat () const
	{
		const auto margin = [this] (const QByteArray& orient) { return XSM_->property (orient + "Margin").toInt (); };
		QTextFrameFormat fmt;
		fmt.setLeftMargin (margin ("Left"));
		fmt.setRightMargin (margin ("Right"));
		fmt.setTopMargin (margin ("Top"));
		fmt.setBottomMargin (margin ("Bottom"));
		fmt.setWidth (XSM_->property ("PageWidth").toInt ());
		return fmt;
	}

	namespace
	{
		std::optional<int> GetHeadingLevel (QStringView tagName)
		{
			if (tagName.size () != 2 || tagName [0] != 'h')
				return {};

			auto level = tagName [1].toLatin1 () - '0';
			if (level < 1 || level > 6)
				return {};
			return level;
		}
	}

	BlockFormat TextDocumentFormatConfig::GetBlockFormat (QStringView tagName, QStringView klass) const
	{
		if (tagName == u"p"_qsv)
			return { { .Align_ = Qt::AlignJustify, .Indent_ = 15 }, {} };

		if (const auto& heading = GetHeadingLevel (tagName))
		{
			BlockOnlyFormat bf;
			bf.HeadingLevel_ = *heading;
			if (*heading <= 2)
				bf.Align_ = Qt::AlignHCenter;

			const auto& defFont = XSM_->property ("DefaultFont").value<QFont> ();

			CharFormat cf
			{
				.PointSize_ = defFont.pointSize () * (2. - *heading / 10.),
			};
			return { bf, cf };
		}

		return {};
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