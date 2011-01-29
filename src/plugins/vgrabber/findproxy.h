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

#ifndef PLUGINS_VGRABBER_FINDPROXY_H
#define PLUGINS_VGRABBER_FINDPROXY_H
#include <boost/optional.hpp>
#include <QAbstractItemModel>
#include <QList>
#include <QUrl>
#include <interfaces/structures.h>
#include <interfaces/ifinder.h>

class QToolBar;
class QAction;

namespace LeechCraft
{
	struct Entity;

	namespace Plugins
	{
		namespace vGrabber
		{
			class CategoriesSelector;

			class FindProxy : public QAbstractItemModel
							, public IFindProxy
			{
				Q_OBJECT
				Q_INTERFACES (IFindProxy);

				QList<QObject*> Downloaders_;
			protected:
				QAction *ActionDownload_;
				QAction *ActionHandle_;
				QAction *ActionCopyToClipboard_;
				QToolBar *Toolbar_;
				Request R_;
				QMap<int, QString> Jobs_;
				boost::optional<QString> Error_;
				QMenu *ContextMenu_;
			public:
				enum FindProxyType
				{
					FPTAudio,
					FPTVideo
				};
			protected:
				FindProxyType FindProxyType_;
				CategoriesSelector *CategoriesSelector_;
			public:
				FindProxy (const Request&, CategoriesSelector*, FindProxyType);
				virtual ~FindProxy ();

				void Start ();
				QAbstractItemModel* GetModel ();
				QByteArray GetUniqueSearchID () const;
				QStringList GetCategories () const;

				virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
			protected:
				void SetError (const QString&);
				virtual QUrl GetURL () const = 0;
				virtual void Handle (const QString&) = 0;
				void EmitWith (TaskParameter, const QUrl&);
				void HandleProvider (QObject*);
			protected slots:
				virtual void handleDownload () = 0;
				virtual void handleHandle () = 0;
				virtual void handleCopyToClipboard ();
			private slots:
				void handleJobFinished (int);
				void handleJobError (int);
			signals:
				void gotEntity (const LeechCraft::Entity&);
				void delegateEntity (const LeechCraft::Entity&,
						int*, QObject**);
				void error (const QString&);
			};

			typedef boost::shared_ptr<FindProxy> FindProxy_ptr;
		};
	};
};

#endif

