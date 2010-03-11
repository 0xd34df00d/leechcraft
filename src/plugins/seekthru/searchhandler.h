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

#ifndef PLUGINS_SEEKTHRU_SEARCHHANDLER_H
#define PLUGINS_SEEKTHRU_SEARCHHANDLER_H
#include <QAbstractItemModel>
#include <boost/shared_ptr.hpp>
#include <QUrl>
#include <interfaces/ifinder.h>
#include <interfaces/structures.h>
#include "description.h"

class QToolBar;
class QAction;

namespace LeechCraft
{
	namespace Util
	{
		class SelectableBrowser;
	};

	namespace Plugins
	{
		namespace SeekThru
		{
			/** This class performs search on a single category with a single search
			 * provider.
			 */
			class SearchHandler : public QAbstractItemModel
			{
				Q_OBJECT

				static const QString OS_;

				Description D_;

				QString SearchString_;
				struct Result
				{
					enum Type
					{
						TypeRSS,
						TypeAtom,
						TypeHTML
					};

					Type Type_;
					int TotalResults_;
					int StartIndex_;
					int ItemsPerPage_;
					QString Response_;
					QString Filename_;
					QUrl RequestURL_;
				};

				QList<Result> Results_;
				QMap<int, Result> Jobs_;
				QList<QObject*> Downloaders_;
				boost::shared_ptr<Util::SelectableBrowser> Viewer_;
				boost::shared_ptr<QToolBar> Toolbar_;
				boost::shared_ptr<QAction> Action_;
			public:
				SearchHandler (const Description&);

				virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
				virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
				virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

				void Start (const LeechCraft::Request&);
			private slots:
				void handleJobFinished (int);
				void handleJobError (int);
				void subscribe ();
			private:
				void HandleProvider (QObject*);
			signals:
				void delegateEntity (const LeechCraft::DownloadEntity&,
						int*, QObject**);
				void gotEntity (const LeechCraft::DownloadEntity&);
				void error (const QString&);
				void warning (const QString&);
			};

			typedef boost::shared_ptr<SearchHandler> SearchHandler_ptr;
		};
	};
};

#endif

