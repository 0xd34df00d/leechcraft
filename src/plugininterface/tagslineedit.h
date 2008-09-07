#ifndef TAGSLINEEDIT_H
#define TAGSLINEEDIT_H
#include <QLineEdit>
#include "config.h"

class TagsLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    LEECHCRAFT_API TagsLineEdit (QWidget* = 0);
public slots:
    LEECHCRAFT_API void complete (const QString&);
protected:
    virtual void focusInEvent (QFocusEvent*);
};

#endif

