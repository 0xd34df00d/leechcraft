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

#pragma once

#include <QDialog>
#include "ui_imagedialog.h"

namespace LeechCraft
{
namespace LHTR
{
	class ImageDialog : public QDialog
	{
		Q_OBJECT

		Ui::ImageDialog Ui_;
	public:
		ImageDialog (QWidget* = 0);

		QString GetPath () const;
		QString GetAlt () const;
		int GetWidth () const;
		int GetHeight () const;

		QString GetFloat () const;
	private slots:
		void on_TypeEmbed__toggled (bool);
		void on_Browse__released ();
	};
}
}
