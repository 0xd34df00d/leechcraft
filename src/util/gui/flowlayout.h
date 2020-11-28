/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QLayout>
#include <QStyle>
#include "guiconfig.h"

namespace LC::Util
{
	/** @brief A simple flow layout implementation.
	 *
	 * Flow layout arranges child items in a dynamic wrappable row, much
	 * like QML's GridView.
	 *
	 * @ingroup GuiUtil
	 */
	class UTIL_GUI_API FlowLayout final : public QLayout
	{
		QList<QLayoutItem*> ItemList_;
		int HSpace_;
		int VSpace_;
	public:
		explicit FlowLayout (QWidget*, int = -1, int = -1, int = -1);
		explicit FlowLayout (int = -1, int = -1, int = -1);
		~FlowLayout () override;

		void addItem (QLayoutItem*) override;

		int horizontalSpacing () const;
		int verticalSpacing () const;

		Qt::Orientations expandingDirections () const override;
		bool hasHeightForWidth () const override;
		int heightForWidth (int) const override;
		int count () const override;
		QLayoutItem* itemAt (int) const override;
		QLayoutItem* takeAt (int) override;
		QSize minimumSize () const override;
		void setGeometry (const QRect&) override;
		QSize sizeHint () const override;
	private:
		int DoLayout (const QRect&, bool) const;
		int SmartSpacing (QStyle::PixelMetric) const;
	};
}
