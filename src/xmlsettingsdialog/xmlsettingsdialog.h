#ifndef XMLSETTINGSDIALOG_H
#define XMLSETTINGSDIALOG_H
#include <QDialog>
#include <QString>

class QStackedWidget;
class QListWidget;
class QPushButton;
class QDomElement;

class XmlSettingsDialog : public QDialog
{
	Q_OBJECT

	QPushButton *OK_, *Cancel_;
	QStackedWidget *Pages_;
	QListWidget *Sections_;
public:
	XmlSettingsDialog (QWidget *parent = 0);
	void RegisterObject (QObject*, const QString&);
private:
	void ParsePage (const QDomElement&);
	void ParseItem (const QDomElement&, QWidget*);
	QString GetLabel (const QDomElement&);
private slots:
	void updatePreferences ();
};

#endif

