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

#ifndef PLUGINS_LMP_CORE_H
#define PLUGINS_LMP_CORE_H
#include <memory>
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>
#include "phonon.h"
#include "player.h"

namespace Phonon
{
	class VideoWidget;
	class SeekSlider;
	class VolumeSlider;
};

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LMP
		{
			class DefaultWidget;

			class Core : public QObject
			{
				Q_OBJECT

				std::auto_ptr<Player> Player_;
				ICoreProxy_ptr Proxy_;
				QAction *ShowAction_;
				mutable DefaultWidget *DefaultWidget_;

				Core ();
			public:
				static Core& Instance ();
				void Release ();
				void SetCoreProxy (ICoreProxy_ptr);
				ICoreProxy_ptr GetCoreProxy () const;

				PlayerWidget* CreateWidget () const;
				IVideoWidget* GetDefaultWidget () const;

				void Reinitialize ();
				void Play ();
				void Pause ();
				void Enqueue (const QUrl&);
				void Enqueue (QIODevice*);
				QAction* GetShowAction () const;
				void Handle (const LeechCraft::DownloadEntity&);
			signals:
				void bringToFront ();
			};
		};
	};
};

#endif

