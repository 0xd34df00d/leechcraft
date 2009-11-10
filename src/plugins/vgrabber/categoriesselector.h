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

#ifndef PLUGINS_VGRABBER_CATEGORIESSELECTOR_H
#define PLUGINS_VGRABBER_CATEGORIESSELECTOR_H
#include <QWidget>
#include "ui_categoriesselector.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace vGrabber
		{
			class vGrabber;

			class CategoriesSelector : public QWidget
			{
				Q_OBJECT

				Ui::CategoriesSelector Ui_;
				vGrabber *Parent_;
				QStringList Deleted_;
				QStringList Added_;
			public:
				enum Type
				{
					TAudio,
					TVideo
				};
			private:
				Type Type_;
			public:
				CategoriesSelector (Type, vGrabber*, QWidget* = 0);

				QStringList GetCategories () const;
				QStringList GetHRCategories () const;
			private:
				void ReadSettings ();
				void WriteSettings ();
				void AddItem (const QString&);
			public slots:
				void accept ();
				void reject ();
			private slots:
				void on_Add__released ();
				void on_Modify__released ();
				void on_Remove__released ();
			signals:
				void goingToAccept (const QStringList& added,
						const QStringList& removed);
			};
		};
	};
};

#endif

