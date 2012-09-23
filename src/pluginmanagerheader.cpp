/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Minh Ngo
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
#include "pluginmanagerheader.h"
#include <QCheckBox>

namespace LeechCraft
{
	const int XPadding_ = 2;
	const int YPadding_ = -8;
	
	PluginManagerHeader::PluginManagerHeader (QWidget *parent)
	: QHeaderView (Qt::Horizontal, parent)
	, SelectAllCheckBox_ (new QCheckBox (this))
	{
		setResizeMode (Interactive);
		QRect rect (SelectAllCheckBox_->geometry ());
		rect.setX (rect.x () + XPadding_);
		rect.setY (rect.y () + YPadding_);
		SelectAllCheckBox_->setGeometry (rect);
		SelectAllCheckBox_->setTristate (true);
		
		connect (SelectAllCheckBox_,
				SIGNAL (stateChanged (int)),
				this,
				SLOT (onSelectAllCheckBoxClicked (int)));
	}
	
	void PluginManagerHeader::onSelectAllCheckBoxClicked (int state)
	{
		if (state == Qt::PartiallyChecked)
			SelectAllCheckBox_->setCheckState (Qt::Checked);
		else
			emit needSelectAll (state);
	}
	
	void PluginManagerHeader::selectAll (Qt::CheckState state)
	{
		SelectAllCheckBox_->disconnect (this, SLOT (onSelectAllCheckBoxClicked (int)));
		SelectAllCheckBox_->setCheckState (state);
		
		connect (SelectAllCheckBox_,
				SIGNAL (stateChanged (int)),
				this,
				SLOT (onSelectAllCheckBoxClicked (int)));
	}
}
