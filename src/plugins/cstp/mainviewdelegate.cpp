/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "mainviewdelegate.h"
#include <QApplication>
#include <plugininterface/proxy.h>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CSTP
		{
			MainViewDelegate::MainViewDelegate (QWidget *parent)
			: QItemDelegate (parent)
			{
			}
			
			void MainViewDelegate::paint (QPainter *painter,
					const QStyleOptionViewItem& option,
					const QModelIndex& index) const
			{
				if (index.column () != Core::HProgress)
				{
					QItemDelegate::paint (painter, option, index);
					return;
				}
				
				QStyleOptionProgressBar pbo;
				pbo.state = QStyle::State_Enabled;
				pbo.direction = QApplication::layoutDirection ();
				pbo.rect = option.rect;
				pbo.fontMetrics = QApplication::fontMetrics ();
				pbo.minimum = 0;
				pbo.maximum = 100;
				pbo.textAlignment = Qt::AlignCenter;
				pbo.textVisible = true;
			
				bool isr = Core::Instance ().IsRunning (index.row ());
			
				if (isr)
				{
					qint64 done = Core::Instance ().GetDone (index.row ()),
						   total = Core::Instance ().GetTotal (index.row ());
					int progress = total ? done * 100 / total : 0;
					pbo.progress = progress;
					pbo.text = QString ("%1 (%2 of %3)")
						.arg (progress)
						.arg (LeechCraft::Util::Proxy::Instance ()->MakePrettySize (done))
						.arg (LeechCraft::Util::Proxy::Instance ()->MakePrettySize (total));
				}
				else
				{
					pbo.progress = 0;
					pbo.text = QString (tr ("Idle"));
				}
			}
		};
	};
};

