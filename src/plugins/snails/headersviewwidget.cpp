/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "headersviewwidget.h"

namespace LC
{
namespace Snails
{
	HeadersViewWidget::HeadersViewWidget (const QByteArray& header, QWidget *parent)
	: QWidget { parent }
	{
		Ui_.setupUi (this);

		const auto& bufferStr = QString::fromUtf8 (header);
		const auto& escaped = bufferStr.toHtmlEscaped ();
		Ui_.Edit_->setHtml ("<pre>" + escaped + "</pre>");
	}
}
}
