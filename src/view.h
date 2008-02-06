#ifndef VIEW_H
#define VIEW_H
#include <QTableView> 
class QHeaderView;

/** @class View view.h
 * @brief Represents data on the screen.
 * @author 0xd34df00d
 * This class represents the download list on the screen, nothing
 * more.
 */
class View : public QTableView
{
    Q_OBJECT
public:
    View (QWidget *parent = 0);
    void DoResizes (int size);

    enum UserInputActions
    {
        CellClick
        , CellDoubleClick
    };
};

#endif

