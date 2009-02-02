#ifndef CATEGORYSELECTOR_H
#define CATEGORYSELECTOR_H
#include <QWidget>
#include "config.h"

class QStringList;
class QString;

namespace LeechCraft
{
	namespace Util
	{
		/** @brief The CategorySelector widget provides a way to select amongst
		 * a group of items.
		 *
		 * The CategorySelector is a QWidget having Qt::Tool window hint. That
		 * results in representing this widget as a tool window - usually a small
		 * window with smaller than usual title bar and decoration.
		 * CategorySelector represents the possible selections as a list of
		 * check boxes.
		 *
		 * Programmer can set the list of possible choice variants using
		 * SetPossibleSelections and get selected items with GetSelections.
		 *
		 * CategorySelector emits selectionChanged() signal when user changes
		 * his selection. CategorySelector's primary purpose is to help user to
		 * select tags using a line edit, so there's a convenience slot
		 * lineTextChanged() which could be used to notify CategorySelector
		 * about changes of possible categories. There are also convenience
		 * slots selectAll() and selectNone() which could be used to mark all
		 * and no elements in the list respectively.
		 */
		class LEECHCRAFT_API CategorySelector : public QWidget
		{
			Q_OBJECT
		public:
			/** @brief Constructor.
			 * 
			 * @param[in] parent Pointer to parent widget.
			 */
			CategorySelector (QWidget *parent = 0);

			/** @brief Sets possible selections.
			 *
			 * Clears previous selections list, sets new possible selections
			 * according to selections parameter. By default, no items are
			 * selected.
			 *
			 * @param[in] selections Possible selections.
			 *
			 * @sa GetSelections
			 */
			void SetPossibleSelections (const QStringList& selections);
			/** @brief Gets selected items.
			 *
			 * Returns the selected items - a subset of selection variants
			 * passed via SetPossibleSelections.
			 *
			 * @return Selected items.
			 *
			 * @sa SetPossibleSelections
			 */
			QStringList GetSelections ();
		protected:
			virtual void moveEvent (QMoveEvent*);
		public slots:
			/** @brief Selects all variants.
			 */
			void selectAll ();
			/** @brief Deselects all variants.
			 */
			void selectNone ();
			/** @brief Notifies CategorySelector about logical selection
			 * changes.
			 *
			 * This slot is usually used to notify CategorySelector about
			 * selection changes done via a related widget - for example, a line
			 * edit with tags.
			 *
			 * @param[in] newText The text of the line edit.
			 */
			void lineTextChanged (const QString& newText);
		private slots:
			void buttonToggled ();
		signals:
			/** @brief Indicates that selections have changed.
			 *
			 * @param[out] newSelections Selected items.
			 */
			void selectionChanged (const QStringList& newSelections);
		};
	};
};

#endif

