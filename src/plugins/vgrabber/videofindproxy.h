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

#ifndef PLUGINS_VGRABBER_VIDEOFINDPROXY_H
#define PLUGINS_VGRABBER_VIDEOFINDPROXY_H
#include "findproxy.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace vGrabber
		{
			class VideoFindProxy : public FindProxy
			{
				Q_OBJECT

				struct VideoResult
				{
					QUrl URL_;
					QString Title_;
					/*
					QString Length_;
					QString Date_;
					QString Description_;
					*/
				};
				QList<VideoResult> VideoResults_;

				enum ProcessingType
				{
					PTInvalid,
					PTDownload,
					PTHandle
				};
				ProcessingType Type_;

				QMap<int, QString> VideoJobs_;
			public:
				VideoFindProxy (const Request&);

				virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
			protected:
				virtual QUrl GetURL () const;
				virtual void Handle (const QString&);
			private:
				void HandleSearchResults (const QString&);
				void HandleVideoPage (const QString&);
				void HandleAction ();
			protected slots:
				virtual void handleDownload ();
				virtual void handleHandle ();
			};
		};
	};
};

#endif

