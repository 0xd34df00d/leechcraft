#include <QtDebug>
#include <QListWidgetItem>
#include "columnselector.h"

ColumnSelector::ColumnSelector (QWidget *parent)
: QDialog (parent)
{
    setupUi (this);
}

void ColumnSelector::SetColumnsStates (const QList<QPair<QString, bool> >& states)
{
    for (int i = 0; i < states.size (); ++i)
    {
        QListWidgetItem *item = new QListWidgetItem (ColumnsSelector_);
        item->setText (states.at (i).first);
        item->setCheckState (states.at (i).second ? Qt::Checked : Qt::Unchecked);
    }
}

QList<bool> ColumnSelector::GetColumnsStates () const
{
    QList<bool> result;
    for (int i = 0; i < ColumnsSelector_->count (); ++i)
        result.append (ColumnsSelector_->item (i)->checkState () == Qt::Checked);
    return result;
}

void ColumnSelector::on_SelectAll__released () const
{
    for (int i = 0; i < ColumnsSelector_->count (); ++i)
        ColumnsSelector_->item (i)->setCheckState (Qt::Checked);
}

void ColumnSelector::on_DeselectAll__released () const
{
    for (int i = 0; i < ColumnsSelector_->count (); ++i)
        ColumnsSelector_->item (i)->setCheckState (Qt::Unchecked);
}

