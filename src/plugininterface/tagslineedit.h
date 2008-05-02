#ifndef TAGSLINEEDIT_H
#define TAGSLINEEDIT_H
#include <QLineEdit>

class TagsLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    TagsLineEdit (QWidget* = 0);
public slots:
    void complete (const QString&);
protected:
    virtual void focusInEvent (QFocusEvent*);
};

#endif

