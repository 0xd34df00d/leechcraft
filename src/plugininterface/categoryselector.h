#ifndef CATEGORYSELECTOR_H
#define CATEGORYSELECTOR_H
#include <QWidget>
#include "config.h"

class QStringList;
class QString;

class CategorySelector : public QWidget
{
	Q_OBJECT
public:
	LEECHCRAFT_API CategorySelector (QWidget* = 0);

	LEECHCRAFT_API void SetPossibleSelections (const QStringList&);
	LEECHCRAFT_API QStringList GetSelections ();
public slots:
	void lineTextChanged (const QString&);
private slots:
	void buttonToggled ();
signals:
	LEECHCRAFT_API void selectionChanged (const QStringList&);
};

#endif

