/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#ifndef PLUGINS_AUSCRIE_AUSCRIE_H
#define PLUGINS_AUSCRIE_AUSCRIE_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/itoolbarembedder.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Auscrie
		{
			class ShooterDialog;

			class Plugin : public QObject
						 , public IInfo
						 , public IToolBarEmbedder
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IToolBarEmbedder)

				ICoreProxy_ptr Proxy_;
				QAction *ShotAction_;
				ShooterDialog *Dialog_;
			public:
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				void Release ();
				QString GetName () const;
				QString GetInfo () const;
				QIcon GetIcon () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);
				QList<QAction*> GetActions () const;
			private slots:
				void makeScreenshot ();
				void shoot ();
				void handleFinished (QNetworkReply*);
				void handleError (QNetworkReply*);
			private:
				void Post (const QByteArray&);
			signals:
				void notify (const LeechCraft::Notification&);
			};
		};
	};
};

#endif

