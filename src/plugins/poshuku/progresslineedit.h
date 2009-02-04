#ifndef PROGRESSLINEEDIT_H
#define PROGRESSLINEEDIT_H
#include <QLineEdit>

class QModelIndex;

class ProgressLineEdit : public QLineEdit
{
	Q_OBJECT

	bool IsCompleting_;
public:
	ProgressLineEdit (QWidget* = 0);
	virtual ~ProgressLineEdit ();
	bool IsCompleting () const;
protected:
	virtual void focusInEvent (QFocusEvent*);
	virtual void keyPressEvent (QKeyEvent*);
public slots:
	void setValue (int);
	void handleActivated (const QModelIndex&);
	void handleHighlighted (const QModelIndex&);
};

#endif

