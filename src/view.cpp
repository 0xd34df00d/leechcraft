#include <iostream>
#include <QHeaderView>
#include "core.h"
#include "view.h"

/** @brief Default constructor.
 * Initializes the object and does some UI-related things.
 *
 * @param parent pointer to the parent QWidget.
 */
View::View (QWidget *parent)
: QTableView (parent)
{
    QHeaderView *hh = horizontalHeader ();
    hh->setSortIndicatorShown (false);
    hh->setStretchLastSection (true);
    hh->setMovable (true);
    
    setShowGrid (false);
    setEditTriggers (QAbstractItemView::DoubleClicked);
    setSelectionBehavior (QAbstractItemView::SelectRows);
    setSelectionMode (QAbstractItemView::SingleSelection);
    setAlternatingRowColors (true);
}

/** @brief Resizes the columns.
 * Resizes the columns according to the given size.
 *
 * @param size overall size.
 */
void View::DoResizes (int size)
{
    QHeaderView *hh = horizontalHeader ();
    hh->resizeSection (0, static_cast<int> (static_cast<double> (size) / 2));
    hh->resizeSection (1, static_cast<int> (static_cast<double> (size) / 2));
}

