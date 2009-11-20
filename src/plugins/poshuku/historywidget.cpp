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

#include "historywidget.h"
#include <QDateTime>
#include "core.h"
#include "historymodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			HistoryWidget::HistoryWidget (QWidget *parent)
			: QWidget (parent)
			{
				Ui_.setupUi (this);

				HistoryFilterModel_.reset (new HistoryFilterModel (this));
				HistoryFilterModel_->setSourceModel (Core::Instance ().GetHistoryModel ());
				Core::Instance ().GetHistoryModel ()->setParent (HistoryFilterModel_.get ());
				HistoryFilterModel_->setDynamicSortFilter (true);
				Ui_.HistoryView_->setModel (HistoryFilterModel_.get ());
			
				connect (Ui_.HistoryFilterLine_,
						SIGNAL (textChanged (const QString&)),
						this,
						SLOT (updateHistoryFilter ()));
				connect (Ui_.HistoryFilterType_,
						SIGNAL (currentIndexChanged (int)),
						this,
						SLOT (updateHistoryFilter ()));
				connect (Ui_.HistoryFilterCaseSensitivity_,
						SIGNAL (stateChanged (int)),
						this,
						SLOT (updateHistoryFilter ()));

				QHeaderView *itemsHeader = Ui_.HistoryView_->header ();
				QFontMetrics fm = fontMetrics ();
				itemsHeader->resizeSection (0,
						fm.width ("Average site title can be very big, it's also the "
							"most important part, so it's priority is the biggest."));
				itemsHeader->resizeSection (1,
						fm.width (QDateTime::currentDateTime ().toString () + " space"));
				itemsHeader->resizeSection (2,
						fm.width ("Average URL could be very very long, but we don't account this."));
			}

			void HistoryWidget::on_HistoryView__activated (const QModelIndex& index)
			{
				if (!index.parent ().isValid ())
					return;

				Core::Instance ().NewURL (index.sibling (index.row (),
							HistoryModel::ColumnURL).data ().toString ());
			}
			
			void HistoryWidget::updateHistoryFilter ()
			{
				int section = Ui_.HistoryFilterType_->currentIndex ();
				QString text = Ui_.HistoryFilterLine_->text ();
			
				switch (section)
				{
					case 1:
						HistoryFilterModel_->setFilterWildcard (text);
						break;
					case 2:
						HistoryFilterModel_->setFilterRegExp (text);
						break;
					default:
						HistoryFilterModel_->setFilterFixedString (text);
						break;
				}
			
				HistoryFilterModel_->
					setFilterCaseSensitivity ((Ui_.HistoryFilterCaseSensitivity_->
								checkState () == Qt::Checked) ? Qt::CaseSensitive :
							Qt::CaseInsensitive);
			}
		};
	};
};

