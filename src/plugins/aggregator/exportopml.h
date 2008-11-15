#ifndef EXPORTOPML_H
#define EXPORTOPML_H
#include <QDialog>
#include "ui_exportopml.h"
#include "channel.h"

class ExportOPML : public QDialog
{
	Q_OBJECT

	Ui::ExportOPML Ui_;
public:
	ExportOPML ();
	virtual ~ExportOPML ();

	QString GetDestination () const;
	QString GetTitle () const;
	QString GetOwner () const;
	QString GetOwnerEmail () const;
	std::vector<bool> GetSelectedFeeds () const;

	void SetFeeds (const channels_shorts_t&);
private slots:
	void on_File__textEdited (const QString&);
	void on_Browse__released ();
};

#endif

