/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#pragma once

#include <QDialog>
#include <interfaces/lmp/ilmpproxy.h>
#include <interfaces/lmp/mediainfo.h>
#include "ui_renamedialog.h"

class QStandardItemModel;

namespace LeechCraft
{
namespace LMP
{
namespace Graffiti
{
	class RenameDialog : public QDialog
	{
		Q_OBJECT

		const ILMPProxy_Ptr Proxy_;
		decltype (ILMPProxy_Ptr ()->GetSubstGetters ()) Getters_;

		Ui::RenameDialog Ui_;

		QStandardItemModel *PreviewModel_;

		QList<QPair<MediaInfo, QString>> Infos_;
	public:
		RenameDialog (ILMPProxy_Ptr, QWidget* = 0);

		void SetInfos (const QList<MediaInfo>&);
	private:
		QList<QPair<QString, QString>> GetRenames () const;
		void Rename (const QList<QPair<QString, QString>>&);
	public slots:
		void accept ();
	private slots:
		void updatePreview ();
	};
}
}
}
