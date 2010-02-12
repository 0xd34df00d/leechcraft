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

#ifndef PLUGINS_CSTP_CSTP_H
#define PLUGINS_CSTP_CSTP_H
#include <memory>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <interfaces/ijobholder.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/structures.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

class QTabWidget;
class QToolBar;
class QModelIndex;
class QTranslator;

namespace boost
{
	namespace logic
	{
		class tribool;
	};
};

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CSTP
		{
			class Core;

			class CSTP : public QObject
					   , public IInfo
					   , public IDownload
					   , public IJobHolder
					   , public IHaveSettings
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IDownload IJobHolder IHaveSettings)

				QMenu *Plugins_;
				std::auto_ptr<QTranslator> Translator_;
				boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> XmlSettingsDialog_;
				std::auto_ptr<QToolBar> Toolbar_;
			public:
				virtual ~CSTP ();
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				void Release ();
				QString GetName () const;
				QString GetInfo () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);
				QIcon GetIcon () const;

				qint64 GetDownloadSpeed () const;
				qint64 GetUploadSpeed () const;
				void StartAll ();
				void StopAll ();
				bool CouldDownload (const LeechCraft::DownloadEntity&) const;
				int AddJob (LeechCraft::DownloadEntity);
				void KillTask (int);

				QAbstractItemModel* GetRepresentation () const;

				boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> GetSettingsDialog () const;
			private:
				template<typename T> void ApplyCore2Selection (void (Core::*) (const QModelIndex&), T);
				void SetupToolbar ();
			private slots:
				void handleItemSelected (const QModelIndex&);
				void handleFileExists (boost::logic::tribool*);
				void handleError (const QString&);
			signals:
				void jobFinished (int);
				void jobRemoved (int);
				void jobError (int, IDownload::Error);
				void gotEntity (const LeechCraft::DownloadEntity&);
				void notify (const LeechCraft::Notification&);
			};
		};
	};
};

#endif

