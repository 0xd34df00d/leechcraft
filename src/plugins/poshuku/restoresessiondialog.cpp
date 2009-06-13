#include "restoresessiondialog.h"
#include <QtDebug>
#include <QHeaderView>
#include <QUrl>
#include <QWebSettings>

using namespace LeechCraft::Plugins::Poshuku;

RestoreSessionDialog::RestoreSessionDialog (QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);

	QHeaderView *header = Ui_.Pages_->header ();
	header->resizeSection (0,
			fontMetrics ().width ("This is an averate web site name, "
				"with stuff from the Web 2.0 era"));
}

RestoreSessionDialog::~RestoreSessionDialog ()
{
}

void RestoreSessionDialog::AddPair (const QString& title,
		const QString& url)
{
	QTreeWidgetItem *item = new QTreeWidgetItem (Ui_.Pages_,
			QStringList (title) << url);
	item->setData (0, Qt::CheckStateRole, Qt::Checked);
	// Do not remote this debugging output, for some reason QWebSettings
	// returns a valid icon only in a second or third call to the DB.
	QWebSettings::iconForUrl (QUrl (url));
	QWebSettings::iconForUrl (QUrl (url)).isNull ();
	item->setIcon (0, QWebSettings::iconForUrl (QUrl (url)));
}

QStringList RestoreSessionDialog::GetSelectedURLs () const
{
	QStringList result;
	for (int i = 0, end = Ui_.Pages_->topLevelItemCount ();
			i < end; ++i)
		if (Ui_.Pages_->topLevelItem (i)->
				data (0, Qt::CheckStateRole).toInt () == Qt::Checked)
			result << Ui_.Pages_->topLevelItem (i)->
				data (1, Qt::DisplayRole).toString ();
	return result;
}

void RestoreSessionDialog::on_SelectAll__released ()
{
	for (int i = 0, end = Ui_.Pages_->topLevelItemCount ();
			i < end; ++i)
		Ui_.Pages_->topLevelItem (i)->setData (0, Qt::CheckStateRole,
				Qt::Checked);
}

void RestoreSessionDialog::on_SelectNone__released ()
{
	for (int i = 0, end = Ui_.Pages_->topLevelItemCount ();
			i < end; ++i)
		Ui_.Pages_->topLevelItem (i)->setData (0, Qt::CheckStateRole,
				Qt::Unchecked);
}


