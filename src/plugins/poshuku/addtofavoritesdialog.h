#ifndef ADDTOFAVORITESDIALOG_H
#define ADDTOFAVORITESDIALOG_H
#include <QDialog>
#include "ui_addtofavoritesdialog.h"

class AddToFavoritesDialog : public QDialog
{
	Q_OBJECT

	Ui::AddToFavoritesDialog Ui_;
public:
	AddToFavoritesDialog (const QString&,
			const QString&, QWidget* = 0);
	virtual ~AddToFavoritesDialog ();

	QString GetTitle () const;
	QStringList GetTags () const;
};

#endif

