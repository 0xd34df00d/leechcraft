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
#include <QPointer>
#include "pasteservicefactory.h"
#include "pasteservicebase.h"
#include "ui_pastedialog.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Autopaste
{
	class PasteDialog : public QDialog
	{
		Q_OBJECT

		Ui::PasteDialog Ui_;
	public:
		enum Choice
		{
			Yes,
			No,
			Cancel
		};
	private:
		Choice Choice_;
	public:
		PasteDialog (QWidget* = 0);

		Choice GetChoice () const;

		PasteServiceFactory::Creator_f GetCreator () const;
		Highlight GetHighlight () const;
	private slots:
		void on_ButtonBox__clicked (QAbstractButton*);
	};
}
}
}
