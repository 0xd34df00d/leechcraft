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

#include "restoresessiondialog.h"
#include <QtDebug>
#include <QHeaderView>
#include <QUrl>
#include <QWebSettings>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			RestoreSessionDialog::RestoreSessionDialog (QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
			
				QHeaderView *header = Ui_.Pages_->header ();
				header->resizeSection (0,
						fontMetrics ().width ("This is an averate web site name, "
							"with stuff from the Web 2.0 era"));
			}
			
			RestoreSessionDialog::~RestoreSessionDialog ()
			{
			}
			
			void RestoreSessionDialog::AddPair (const QString& title,
					const QString& url)
			{
				QTreeWidgetItem *item = new QTreeWidgetItem (Ui_.Pages_,
						QStringList (title) << url);
				item->setData (0, Qt::CheckStateRole, Qt::Checked);
				// Do not remote this debugging output, for some reason QWebSettings
				// returns a valid icon only in a second or third call to the DB.
				item->setIcon (0, Core::Instance ().GetIcon (QUrl (url)));
			}
			
			QStringList RestoreSessionDialog::GetSelectedURLs () const
			{
				QStringList result;
				for (int i = 0, end = Ui_.Pages_->topLevelItemCount ();
						i < end; ++i)
					if (Ui_.Pages_->topLevelItem (i)->
							data (0, Qt::CheckStateRole).toInt () == Qt::Checked)
						result << Ui_.Pages_->topLevelItem (i)->
							data (1, Qt::DisplayRole).toString ();
				return result;
			}
			
			void RestoreSessionDialog::on_SelectAll__released ()
			{
				for (int i = 0, end = Ui_.Pages_->topLevelItemCount ();
						i < end; ++i)
					Ui_.Pages_->topLevelItem (i)->setData (0, Qt::CheckStateRole,
							Qt::Checked);
			}
			
			void RestoreSessionDialog::on_SelectNone__released ()
			{
				for (int i = 0, end = Ui_.Pages_->topLevelItemCount ();
						i < end; ++i)
					Ui_.Pages_->topLevelItem (i)->setData (0, Qt::CheckStateRole,
							Qt::Unchecked);
			}
		};
	};
};

