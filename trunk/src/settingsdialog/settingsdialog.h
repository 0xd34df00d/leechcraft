#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H
#include <QDialog>
#include <QObjectList>
#include <QMap>

class QStackedWidget;
class QListWidget;
class QLayout;
class TypeHandler;
class QGroupBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT

 QStackedWidget *Pages_;
 QObjectList Objects_;
 QListWidget *Sections_;
 TypeHandler *TypeHandler_;
 QList<QGroupBox*> GroupBoxes_;
 QPushButton *OK_, *Cancel_;
 QList<QMetaProperty> CurrentProps_;
 QMap<QString, QWidget*> PropertyToParentWidget_;
public:
 SettingsDialog (QWidget *parent = 0);
 virtual ~SettingsDialog ();
 void RegisterObject (QObject*);
 void Rehash ();
 QList<QMetaProperty>& AccessProps ();
 void SetParentWidgetForProperty (const QString&, QWidget*);
public slots:
 void accept ();
private slots:
 void doAdjustSize ();
};

#endif

