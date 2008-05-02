#include <QtDebug>
#include <QCompleter>
#include "tagslineedit.h"

TagsLineEdit::TagsLineEdit (QWidget* parent)
: QLineEdit (parent)
{
}

void TagsLineEdit::complete (const QString& completion)
{
    QString wtext = text ();
    qDebug () << Q_FUNC_INFO << wtext;
    int pos = wtext.lastIndexOf (' ');
    if (pos == -1)
        wtext = completion;
    else
        wtext = wtext.left (pos).append (' ').append (completion);
    setText (wtext);
}

void TagsLineEdit::focusInEvent (QFocusEvent *e)
{
    QLineEdit::focusInEvent (e);
    if (completer ())
    {
        disconnect (completer (), SIGNAL (activated (const QString&)), this, SLOT (setText (const QString&)));
        disconnect (completer (), SIGNAL (highlighted (const QString&)), this, 0);
        connect (completer (), SIGNAL (highlighted (const QString&)), this, SLOT (complete (const QString&)));
    }
}

