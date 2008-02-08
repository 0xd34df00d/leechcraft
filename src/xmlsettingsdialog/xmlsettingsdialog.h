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
class QGridLayout;

class XmlSettingsDialog : public QDialog
{
    Q_OBJECT

    QPushButton *OK_, *Cancel_;
    QStackedWidget *Pages_;
    QListWidget *Sections_;
    QObject *WorkingObject_;
    typedef QMap<QString, QVariant> Property2Value_t;
    Property2Value_t Prop2NewValue_;
    QString DefaultLang_;
public:
    XmlSettingsDialog (QWidget *parent = 0);
    void RegisterObject (QObject*, const QString&);
private:
    void HandleDeclaration (const QDomElement&);
    void ParsePage (const QDomElement&);
    void ParseEntity (const QDomElement&, QWidget*);
    void ParseItem (const QDomElement&, QWidget*);
    QString GetLabel (const QDomElement&);
private:
    void DoLineedit (const QDomElement&, QGridLayout*, QVariant&);
    void DoCheckbox (const QDomElement&, QGridLayout*, QVariant&);
    void DoSpinbox (const QDomElement&, QGridLayout*, QVariant&);
    void DoGroupbox (const QDomElement&, QGridLayout*, QVariant&);
    void DoSpinboxRange (const QDomElement&, QGridLayout*, QVariant&);
    void DoPath (const QDomElement&, QGridLayout*, QVariant&);
    void DoRadio (const QDomElement&, QGridLayout*, QVariant&);
private slots:
    void updatePreferences ();
protected:
    virtual void accept ();
};

#endif

