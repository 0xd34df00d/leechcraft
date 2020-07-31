/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QWidget>
#include <QUrl>
#include "ui_flashplaceholder.h"

class QWebElement;

namespace LC
{
namespace Poshuku
{
namespace FOC
{
	class FlashOnClickWhitelist;

	class FlashPlaceHolder : public QWidget
	{
		Q_OBJECT
		Q_PROPERTY (bool swapping READ IsSwapping)

		FlashOnClickWhitelist * const WL_;

		Ui::FlashPlaceHolder Ui_;
		QUrl URL_;
		bool Swapping_ = false;
	public:
		FlashPlaceHolder (const QUrl&, FlashOnClickWhitelist*, QWidget* = 0);

		bool IsSwapping () const;
	private:
		void PerformWithElements (const std::function<void (QWebElement)>&);
	private slots:
		void handleLoadFlash ();
		void handleHideFlash ();
		void handleContextMenu ();
		void handleAddWhitelist ();
	};
}
}
}
