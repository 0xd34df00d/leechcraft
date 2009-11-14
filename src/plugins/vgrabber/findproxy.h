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

				QList<QObject*> Downloaders_;
			protected:
				QAction *ActionDownload_;
				QAction *ActionHandle_;
				QToolBar *Toolbar_;
				Request R_;
				QMap<int, QString> Jobs_;
			public:
				FindProxy (const Request&);
				virtual ~FindProxy ();

				void Start ();
				QAbstractItemModel* GetModel ();

				virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
			protected:
				virtual QUrl GetURL () const = 0;
				virtual void Handle (const QString&) = 0;
				void EmitWith (TaskParameter, const QUrl&);
				void HandleProvider (QObject*);
			protected slots:
				virtual void handleDownload () = 0;
				virtual void handleHandle () = 0;
			private slots:
				void handleJobFinished (int);
				void handleJobError (int);
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

