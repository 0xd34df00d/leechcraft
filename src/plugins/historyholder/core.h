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

#ifndef PLUGINS_HISTORYHOLDER_CORE_H
#define PLUGINS_HISTORYHOLDER_CORE_H
#include <boost/shared_ptr.hpp>
#include <QAbstractItemModel>
#include <QDateTime>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>
#include <interfaces/ihaveshortcuts.h>

class QToolBar;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace HistoryHolder
		{
			class Core : public QAbstractItemModel
			{
				Q_OBJECT

				Core ();

			public:
				struct HistoryEntry
				{
					LeechCraft::Entity Entity_;
					QDateTime DateTime_;
				};
			private:
				typedef QList<HistoryEntry> History_t;
				History_t History_;
				QStringList Headers_;
				boost::shared_ptr<QToolBar> ToolBar_;
				ICoreProxy_ptr CoreProxy_;
				QAction *Remove_;

				enum Shortcuts
				{
					SRemove
				};
			public:
				static Core& Instance ();
				void Release ();
				void SetCoreProxy (ICoreProxy_ptr);
				ICoreProxy_ptr GetCoreProxy () const;
				void Handle (const LeechCraft::Entity&);

				void SetShortcut (const QString&, const QKeySequences_t&);
				QMap<QString, ActionInfo> GetActionInfo () const;

				int columnCount (const QModelIndex&) const;
				QVariant data (const QModelIndex&, int) const;
				QVariant headerData (int, Qt::Orientation, int) const;
				QModelIndex index (int, int, const QModelIndex&) const;
				QModelIndex parent (const QModelIndex&) const;
				int rowCount (const QModelIndex&) const;
			public slots:
				void handleTasksTreeActivated (const QModelIndex&);
			private:
				void WriteSettings ();
			private slots:
				void remove ();
			signals:
				void gotEntity (const LeechCraft::Entity&);
			};
		};
	};
};

Q_DECLARE_METATYPE (LeechCraft::Plugins::HistoryHolder::Core::HistoryEntry);

#endif

