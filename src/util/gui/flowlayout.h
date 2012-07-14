/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef UTIL_FLOWLAYOUT_H
#define UTIL_FLOWLAYOUT_H
#include <QLayout>
#include <QStyle>
#include <util/utilconfig.h>

namespace LeechCraft
{
namespace Util
{
	class UTIL_API FlowLayout : public QLayout
	{
		QList<QLayoutItem*> ItemList_;
		int HSpace_;
		int VSpace_;
	public:
		FlowLayout (QWidget*, int = -1, int = -1, int = -1);
		FlowLayout (int = -1, int = -1, int = -1);
		virtual ~FlowLayout ();
		
		void addItem (QLayoutItem*);
		int horizontalSpacing () const;
		int verticalSpacing () const;
		Qt::Orientations expandingDirections () const;
		bool hasHeightForWidth () const;
		int heightForWidth (int) const;
		int count () const;
		QLayoutItem* itemAt (int) const;
		QLayoutItem* takeAt (int);
		QSize minimumSize () const;
		void setGeometry (const QRect&);
		QSize sizeHint () const;
	private:
		int DoLayout (const QRect&, bool) const;
		int SmartSpacing (QStyle::PixelMetric) const;
	};
}
}

#endif
