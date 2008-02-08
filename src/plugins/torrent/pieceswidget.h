#ifndef PIECESWIDGET_H
#define PIECESWIDGET_H
#include <QLabel>
#include <vector>

class PiecesWidget : public QLabel
{
    Q_OBJECT

    std::vector<bool> Pieces_;
public:
    PiecesWidget (QWidget *parent = 0);
public slots:
    void setPieceMap (const std::vector<bool>&);
private:
    void paintEvent (QPaintEvent*);
};

#endif

