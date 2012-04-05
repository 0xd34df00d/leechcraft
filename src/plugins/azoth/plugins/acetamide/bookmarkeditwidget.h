/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_BOOKMARKEDITWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_BOOKMARKEDITWIDGET_H
#include <QWidget>
#include <QVariant>
#include <interfaces/azoth/imucbookmarkeditorwidget.h>
#include "ui_bookmarkeditwidget.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class BookmarkEditWidget : public QWidget
							 , public IMUCBookmarkEditorWidget
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IMUCBookmarkEditorWidget);
		
		Ui::BookmarkEditWidget Ui_;
	public:
		BookmarkEditWidget (QWidget* = 0);
		
		QVariantMap GetIdentifyingData () const;
		void SetIdentifyingData (const QVariantMap&);
	};
}
}
}

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_BOOKMARKEDITWIDGET_H
