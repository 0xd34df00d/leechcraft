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
#include <boost/shared_ptr.hpp>
#include "config.h"

class QStackedWidget;
class QListWidget;
class QPushButton;
class QDomElement;
class QFormLayout;
class QDomDocument;

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
	boost::shared_ptr<QDomDocument> Document_;
    struct LangElements
    {
        bool Valid_;
        QPair<bool, QString> Label_;
        QPair<bool, QString> Suffix_;
    };
public:
    LEECHCRAFT_API XmlSettingsDialog (QWidget *parent = 0);
	LEECHCRAFT_API virtual ~XmlSettingsDialog ();
    LEECHCRAFT_API void RegisterObject (QObject*, const QString&);
	LEECHCRAFT_API QString GetXml () const;
	LEECHCRAFT_API void MergeXml (const QByteArray&);
private:
    void HandleDeclaration (const QDomElement&);
    void ParsePage (const QDomElement&);
    void ParseEntity (const QDomElement&, QWidget*);
    void ParseItem (const QDomElement&, QWidget*);
    QString GetLabel (const QDomElement&) const;
    LangElements GetLangElements (const QDomElement&) const;
	QVariant GetValue (const QDomElement&, bool = false) const;
    void DoLineedit (const QDomElement&, QFormLayout*);
    void DoCheckbox (const QDomElement&, QFormLayout*);
    void DoSpinbox (const QDomElement&, QFormLayout*);
    void DoDoubleSpinbox (const QDomElement&, QFormLayout*);
    void DoGroupbox (const QDomElement&, QFormLayout*);
    void DoSpinboxRange (const QDomElement&, QFormLayout*);
    void DoPath (const QDomElement&, QFormLayout*);
    void DoRadio (const QDomElement&, QFormLayout*);
    void DoCombobox (const QDomElement&, QFormLayout*);
	void DoFont (const QDomElement&, QFormLayout*);
	QList<QImage> GetImages (const QDomElement&) const;
	void UpdateXml (bool = false);
	void UpdateSingle (const QString&, const QVariant&, QDomElement&);
private slots:
    void updatePreferences ();
protected:
    virtual void accept ();
    virtual void reject ();
};

#endif

