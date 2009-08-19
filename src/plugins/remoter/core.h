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

#ifndef CORE_H
#define CORE_H
#include <QObjectList>
#include <QDomDocument>
#include <QDomElement>
#include <QVariantList>
#include <QMap>
#include <QString>
#include <QPair>
#include <plugininterface/guarded.h>

class QStringList;
namespace LeechCraft
{
	namespace Util
	{
		class MergeModel;
	};
};

namespace Wt
{
	class WServer;
	class WEnvironment;
	class WApplication;
	class WContainerWidget;
	class WTreeView;
};

class Core : public QObject
{
    Q_OBJECT

	Wt::WServer *Server_;
	LeechCraft::Util::Guarded<LeechCraft::Util::MergeModel*> TasksModel_,
		HistoryModel_;
	QObjectList Objects_;

    Core ();
public:
    static Core& Instance ();
    void Release ();
	void SetHistoryModel (LeechCraft::Util::MergeModel*);
	void SetDownloadersModel (LeechCraft::Util::MergeModel*);
	void SelectedDownloaderChanged (QObject*);
    void AddObject (QObject*, const QString& feature);
	Wt::WApplication* CreateApplication (const Wt::WEnvironment&);
	LeechCraft::Util::MergeModel* GetTasksModel () const;
	LeechCraft::Util::MergeModel* GetHistoryModel () const;
private:
	void InitializeServer ();
};

#endif

