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

#ifndef PLUGINS_LMP_PLAYER_H
#define PLUGINS_LMP_PLAYER_H
#include <memory>
#include <QDialog>
#include <QStandardItemModel>
#include "ui_player.h"
#include "phonon.h"

class QStatusBar;
class QToolBar;
class QAction;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LMP
		{
			class Player : public QDialog
			{
				Q_OBJECT

				Ui::Player Ui_;
				QStatusBar *StatusBar_;
				std::auto_ptr<QStandardItemModel> QueueModel_;
				enum
				{
					SourceRole = Qt::UserRole + 100
				};
			public:
				Player (QWidget* = 0);
				void Play ();
				void Pause ();
				void Stop ();
				void Clear ();
				void TogglePause ();
				void Enqueue (Phonon::MediaSource*);
			private:
				void FillQueue (int) const;
			public slots:
				void handleStateUpdated (const QString&);
			private slots:
				void handleSourceChanged (const Phonon::MediaSource&);
				void handleMetadataChanged ();
				void on_Queue__activated (const QModelIndex&);
			signals:
				void gotEntity (const LeechCraft::DownloadEntity&);
			};
		};
	};
};

#endif

