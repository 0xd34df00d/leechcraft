#ifndef TAGSLINEEDIT_H
#define TAGSLINEEDIT_H
#include <memory>
#include <QLineEdit>
#include "config.h"
#include "categoryselector.h"

/** @brief A line edit class suitable for use with TagsCompleter.
 *
 * One would need this extra class because of custom behavior of both
 * tags completer and line edit semantics.
 *
 * @sa TagsCompleter
 */
class TagsLineEdit : public QLineEdit
{
    Q_OBJECT

	std::auto_ptr<CategorySelector> CategorySelector_;
public:
	/** @brief Constructs the line edit widget.
	 *
	 * Creates the line edit widget.
	 *
	 * @param[in] parent Parent widget.
	 */
    LEECHCRAFT_API TagsLineEdit (QWidget *parent);
	/** @brief Adds the selector widget to the line edit.
	 *
	 * Because this function uses the completion model, it should be
	 * used after the association with a TagsCompleter.
	 *
	 * @sa TagsCompleter
	 */
	LEECHCRAFT_API void AddSelector ();
public slots:
	/** @brief Completes the string.
	 *
	 * Completes the current text in line edit with completion passed
	 * throught string parameter.
	 *
	 * @param[in] string String with completion.
	 */
    LEECHCRAFT_API void complete (const QString& string);
private slots:
	void handleTagsUpdated (const QStringList&);
	void handleSelectionChanged (const QStringList&);
protected:
    virtual void focusInEvent (QFocusEvent*);
	virtual void contextMenuEvent (QContextMenuEvent*);
};

#endif

