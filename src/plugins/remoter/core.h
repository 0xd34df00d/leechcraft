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

