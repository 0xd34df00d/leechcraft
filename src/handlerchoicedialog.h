#ifndef HANDLERCHOICEDIALOG_H
#define HANDLERCHOICEDIALOG_H
#include <map>
#include <memory>
#include <QDialog>
#include <QButtonGroup>
#include "ui_handlerchoicedialog.h"

class IInfo;
class IDownload;
class IEntityHandler;

class HandlerChoiceDialog : public QDialog
{
	Q_OBJECT

	Ui::HandlerChoiceDialog Ui_;
	std::auto_ptr<QButtonGroup> Buttons_;
	typedef std::map<QString, IDownload*> downloaders_t;
	downloaders_t Downloaders_;
	typedef std::map<QString, IEntityHandler*> handlers_t;
	std::map<QString, IEntityHandler*> Handlers_;
public:
	HandlerChoiceDialog (const QString&, QWidget* = 0);

	void Add (const IInfo*, IDownload*);
	void Add (const IInfo*, IEntityHandler*);
	IDownload* GetDownload ();
	IEntityHandler* GetEntityHandler ();
	int NumChoices () const;
};

#endif

