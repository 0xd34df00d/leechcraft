#ifndef COLUMNSELECTOR_H
#define COLUMNSELECTOR_H
#include <QDialog>
#include "ui_columnselector.h"

class ColumnSelector : public QDialog, private Ui::ColumnSelector
{
    Q_OBJECT
public:
    ColumnSelector (QWidget *parent = 0);
    void SetColumnsStates (const QList<QPair<QString, bool> >&);
    QList<bool> GetColumnsStates () const;
private slots:
    void on_SelectAll__released () const;
    void on_DeselectAll__released () const;
};


#endif

