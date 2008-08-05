/*
    Copyright (c) 2008 by Rudoy Georg <0xd34df00d@gmail.com>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/
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
class QFormLayout;

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
    struct LangElements
    {
        bool Valid_;
        QPair<bool, QString> Label_;
        QPair<bool, QString> Suffix_;
    };
public:
    XmlSettingsDialog (QWidget *parent = 0);
    void RegisterObject (QObject*, const QString&);
private:
    void HandleDeclaration (const QDomElement&);
    void ParsePage (const QDomElement&);
    void ParseEntity (const QDomElement&, QWidget*);
    void ParseItem (const QDomElement&, QWidget*);
    QString GetLabel (const QDomElement&) const;
    LangElements GetLangElements (const QDomElement&) const;
private:
    void DoLineedit (const QDomElement&, QFormLayout*, QVariant&);
    void DoCheckbox (const QDomElement&, QFormLayout*, QVariant&);
    void DoSpinbox (const QDomElement&, QFormLayout*, QVariant&);
    void DoGroupbox (const QDomElement&, QFormLayout*, QVariant&);
    void DoSpinboxRange (const QDomElement&, QFormLayout*, QVariant&);
    void DoPath (const QDomElement&, QFormLayout*, QVariant&);
    void DoRadio (const QDomElement&, QFormLayout*, QVariant&);
    void DoCombobox (const QDomElement&, QFormLayout*, QVariant&);
	QList<QImage> GetImages (const QDomElement&) const;
private slots:
    void updatePreferences ();
protected:
    virtual void accept ();
    virtual void reject ();
};

#endif

