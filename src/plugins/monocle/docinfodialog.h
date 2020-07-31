/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QDialog>
#include "ui_docinfodialog.h"

class QStandardItemModel;

namespace LC
{
namespace Monocle
{
	struct FontInfo;

	class IDocument;
	typedef std::shared_ptr<IDocument> IDocument_ptr;

	class DocInfoDialog : public QDialog
	{
		Q_OBJECT

		Ui::DocInfoDialog Ui_;
		QStandardItemModel * const FontsModel_;
	public:
		DocInfoDialog (const QString&, const IDocument_ptr&, QWidget* = 0);
	private slots:
		void HandleFontsInfo (const QList<FontInfo>&);
	};
}
}
