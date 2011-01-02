/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_BITTORRENT_SPEEDSELECTORACTION_H
#define PLUGINS_BITTORRENT_SPEEDSELECTORACTION_H
#include <QWidgetAction>
#include <QComboBox>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class SpeedSelectorAction : public QWidgetAction
			{
				Q_OBJECT

				QString Setting_;
			public:
				SpeedSelectorAction (const QString&, QObject*);

				int CurrentData ();
			protected:
				QWidget* createWidget (QWidget*);
				void deleteWidget (QWidget*);
			public slots:
				void handleSpeedsChanged ();
			private slots:
				void syncSpeeds (int);
			private:
				template<typename F>
				void Call (F f)
				{
					Q_FOREACH (QWidget *w, createdWidgets ())
						f (static_cast<QComboBox*> (w));
				}
			signals:
				void currentIndexChanged (int);
			};
		};
	};
};

#endif

