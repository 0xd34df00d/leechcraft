#ifndef CORE_H
#define CORE_H
#include <QObjectList>
#include <QDomDocument>
#include <QDomElement>
#include <QVariantList>
#include <QMap>
#include <QString>
#include <QPair>

class QStringList;
class MergeModel;

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
	MergeModel *TasksModel_, *HistoryModel_;
	QObjectList Objects_;

    Core ();
public:
    static Core& Instance ();
    void Release ();
	void SetHistoryModel (MergeModel*);
	void SetDownloadersModel (MergeModel*);
    void AddObject (QObject*, const QString& feature);
	Wt::WApplication* CreateApplication (const Wt::WEnvironment&);
private:
	void InitializeServer ();
	void BuildInterface (Wt::WContainerWidget*, const Wt::WEnvironment&);
	void SetupDownloadersView (Wt::WTreeView*);
	void SetupHistoryView (Wt::WTreeView*);
};

#endif

