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

#ifndef PLUGINS_VGRABBER_FINDPROXY_H
#define PLUGINS_VGRABBER_FINDPROXY_H
#include <QAbstractItemModel>
#include <QList>
#include <QUrl>
#include <interfaces/structures.h>
#include <interfaces/ifinder.h>

class QToolBar;
class QAction;

namespace LeechCraft
{
	struct DownloadEntity;

	namespace Plugins
	{
		namespace vGrabber
		{
			class FindProxy : public QAbstractItemModel
							, public IFindProxy
			{
				Q_OBJECT
				Q_INTERFACES (IFindProxy);

				QToolBar *Toolbar_;
				QAction *ActionDownload_;
				QAction *ActionHandle_;

				Request R_;

				QMap<int, QString> Jobs_;

				struct AudioResult
				{
					QUrl URL_;
					int Length_;
					QString Performer_;
					QString Title_;
				};
				QList<AudioResult> AudioResults_;

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

				QList<QObject*> Downloaders_;
			public:
				enum Type
				{
					TAudio,
					TVideo
				};
			private:
				Type Type_;
			public:
				FindProxy (Type, const Request&);
				virtual ~FindProxy ();

				void Start ();
				QAbstractItemModel* GetModel ();

				virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
				virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
				virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
			private slots:
				void handleJobFinished (int);
				void handleJobError (int);
				void handleDownload ();
				void handleHandle ();
			private:
				void HandleAsAudio (const QString&);
				void HandleAsVideo (const QString&);
				void EmitWith (TaskParameter);
				void HandleProvider (QObject*);
				QUrl GetURL () const;
			signals:
				void gotEntity (const LeechCraft::DownloadEntity&);
				void delegateEntity (const LeechCraft::DownloadEntity&,
						int*, QObject**);
				void error (const QString&);
			};

			typedef boost::shared_ptr<FindProxy> FindProxy_ptr;
		};
	};
};

#endif

