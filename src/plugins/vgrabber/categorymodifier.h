/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#ifndef PLUGINS_VGRABBER_CATEGORYMODIFIER_H
#define PLUGINS_VGRABBER_CATEGORYMODIFIER_H
#include <QDialog>
#include "ui_categorymodifier.h"

namespace LeechCraft
{
	namespace Util
	{
		class TagsCompleter;
	};

	namespace Plugins
	{
		namespace vGrabber
		{
			class CategoryModifier : public QDialog
			{
				Q_OBJECT

				Ui::CategoryModifier Ui_;
				Util::TagsCompleter *Completer_;
			public:
				CategoryModifier (const QString&, QWidget* = 0);
				QString GetText () const;
			private slots:
				void on_Line__textChanged (const QString&);
			};
		};
	};
};

#endif

