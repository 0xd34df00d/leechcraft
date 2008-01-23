#ifndef XMLSETTINGSDIALOG_H
#define XMLSETTINGSDIALOG_H
#include <QDialog>
#include <QString>
#include <QMap>
#include <QVariant>

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
	QObject *WorkingObject_;
	typedef QMap<QString, QVariant> Property2Value_t;
	Property2Value_t Prop2NewValue_;
public:
	XmlSettingsDialog (QWidget *parent = 0);
	void RegisterObject (QObject*, const QString&);
private:
	void ParsePage (const QDomElement&);
	void ParseEntity (const QDomElement&, QWidget*);
	void ParseItem (const QDomElement&, QWidget*);
	QString GetLabel (const QDomElement&);
private slots:
	void updatePreferences ();
protected:
	virtual void accept ();
};

#endif

