#ifndef PIECESWIDGET_H
#define PIECESWIDGET_H
#include <QLabel>
#include <vector>
#include <libtorrent/bitfield.hpp>

class PiecesWidget : public QLabel
{
    Q_OBJECT

    libtorrent::bitfield Pieces_;
public:
    PiecesWidget (QWidget *parent = 0);
public slots:
    void setPieceMap (const libtorrent::bitfield&);
private:
    void paintEvent (QPaintEvent*);
};

#endif

