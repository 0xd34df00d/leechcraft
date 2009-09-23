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

#ifndef PLUGINS_AGGREGATOR_STARTUPTHIRDPAGE_H
#define PLUGINS_AGGREGATOR_STARTUPTHIRDPAGE_H
#include <QWizardPage>
#include "ui_startupthirdpage.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class StartupThirdPage : public QWizardPage
			{
				Q_OBJECT

				Ui::StartupThirdPageWidget Ui_;
				struct FeedInfo
				{
					QString Name_;
					QString DefaultTags_;
					QString URL_;

					FeedInfo (const QString&, const QString&, const QString&);
				};
				typedef QList<FeedInfo> FeedInfos_t;
				QMap<QString, FeedInfos_t> Sets_;
			public:
				StartupThirdPage (QWidget* = 0);

				void initializePage ();
			private:
				void Populate (const QString&);
			private slots:
				void handleAccepted ();
				void handleCurrentIndexChanged (const QString&);
			};
		};
	};
};

#endif

