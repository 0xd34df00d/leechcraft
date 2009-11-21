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

#include "summarytab.h"
#include <QTreeView>
#include <QTimer>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			SummaryTab::SummaryTab (QWidget *parent)
			: QWidget (parent)
			{
				Ui_.setupUi (this);
				QTimer *timer = new QTimer (this);
				connect (timer,
						SIGNAL (timeout ()),
						this,
						SLOT (updateTab ()));
				timer->start (1000);
			}

			void SummaryTab::updateTab ()
			{
				if (!Current_.isValid ())
					return;
			}

			void SummaryTab::handleCurrentChanged (const QModelIndex& current)
			{
				if (!current.isValid ())
				{
					Current_ = QModelIndex ();
					return;
				}

				Current_ = Core::Instance ().GetCoreProxy ()->MapToSource (current);
				if (Current_.model () != Core::Instance ().GetModel ())
				{
					Current_ = QModelIndex ();
					return;
				}

				Ui_.DownloadSpeed_->setValue (Core::Instance ().GetModel ()->
						data (Current_, Core::RoleDownSpeedLimit).toInt () / 1024);
				Ui_.UploadSpeed_->setValue (Core::Instance ().GetModel ()->
						data (Current_, Core::RoleUpSpeedLimit).toInt () / 1024);
			}

			void SummaryTab::on_DownloadSpeed__valueChanged (int value)
			{
				Core::Instance ().GetModel ()->
					setData (Current_, value * 1024, Core::RoleDownSpeedLimit);
			}

			void SummaryTab::on_UploadSpeed__valueChanged (int value)
			{
				Core::Instance ().GetModel ()->
					setData (Current_, value * 1024, Core::RoleUpSpeedLimit);
			}
		}
	};
};

