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

#ifndef PLUGINS_BITTORRENT_THIRDSTEP_H
#define PLUGINS_BITTORRENT_THIRDSTEP_H
#include <QWizardPage>
#include "ui_newtorrentthirdstep.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class ThirdStep : public QWizardPage, private Ui::NewTorrentThirdStep
			{
				Q_OBJECT

				quint64 TotalSize_;
			public:
				ThirdStep (QWidget *parent = 0);
				virtual void initializePage ();
			private slots:
				void on_PieceSize__currentIndexChanged ();
			};
		};
	};
};

#endif

