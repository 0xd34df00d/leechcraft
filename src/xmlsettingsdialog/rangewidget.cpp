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
#include <QHBoxLayout>
#include <QVariant>
#include <QSpinBox>
#include "rangewidget.h"

RangeWidget::RangeWidget (QWidget *parent)
: QWidget (parent)
{
    Lower_ = new QSpinBox;
    Higher_ = new QSpinBox;

    QHBoxLayout *lay = new QHBoxLayout;
	lay->setContentsMargins (0, 0, 0, 0);
    lay->addWidget (Lower_);
    lay->addWidget (Higher_);
    lay->addStretch (1);

    connect (Lower_, SIGNAL (valueChanged (int)), this, SLOT (lowerChanged (int)));
    connect (Higher_, SIGNAL (valueChanged (int)), this, SLOT (upperChanged (int)));
    connect (Lower_, SIGNAL (valueChanged (int)), this, SIGNAL (changed ()));
    connect (Higher_, SIGNAL (valueChanged (int)), this, SIGNAL (changed ()));
    
    setLayout (lay);
}

void RangeWidget::SetMinimum (int val)
{
    Lower_->setMinimum (val);
    Higher_->setMinimum (val);
}

void RangeWidget::SetMaximum (int val)
{
    Lower_->setMaximum (val);
    Higher_->setMaximum (val);
}

void RangeWidget::SetLower (int val)
{
    Lower_->setValue (val);
    Higher_->setMinimum (val);
}

void RangeWidget::SetHigher (int val)
{
    Higher_->setValue (val);
    Lower_->setMaximum (val);
}

void RangeWidget::SetRange (const QVariant& variant)
{
    if (!variant.canConvert<QList<QVariant> > ())
        return;

    QList<QVariant> list = variant.toList ();
    if (list.size () != 2)
        return;
    SetLower (list.at (0).toInt ());
    SetHigher (list.at (1).toInt ());
}

QVariant RangeWidget::GetRange () const
{
    QList<QVariant> result;
    result << Lower_->value () << Higher_->value ();
    return result;
}

void RangeWidget::lowerChanged (int val)
{
    Higher_->setMinimum (val);
}

void RangeWidget::upperChanged (int val)
{
    Lower_->setMaximum (val);
}

