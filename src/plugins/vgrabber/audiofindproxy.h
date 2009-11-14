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

#ifndef PLUGINS_VGRABBER_AUDIOFINDPROXY_H
#define PLUGINS_VGRABBER_AUDIOFINDPROXY_H
#include "findproxy.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace vGrabber
		{
			class AudioFindProxy : public FindProxy
			{
				Q_OBJECT

				struct AudioResult
				{
					QUrl URL_;
					int Length_;
					QString Performer_;
					QString Title_;
				};
				QList<AudioResult> AudioResults_;
			public:
				AudioFindProxy (const Request&);

				virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
			protected:
				virtual QUrl GetURL () const;
				virtual void Handle (const QString&);
			protected slots:
				virtual void handleDownload ();
				virtual void handleHandle ();
			};
		};
	};
};

#endif

