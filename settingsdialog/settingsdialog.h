#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H
#include <QDialog>
#include <QObjectList>

class QStackedWidget;
class QComboBox;
class QLayout;
class TypeHandler;
class QGroupBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT

	QStackedWidget *Pages_;
	QObjectList Objects_;
	QComboBox *Combo_;
	TypeHandler *TypeHandler_;
	QList<QGroupBox*> GroupBoxes_;
	QPushButton *OK_, *Cancel_;
public:
	SettingsDialog (QWidget *parent = 0);
	virtual ~SettingsDialog ();
	void RegisterObject (QObject*);
	void Rehash ();
public slots:
	void accept ();
private slots:
	void doAdjustSize ();
};

#endif

