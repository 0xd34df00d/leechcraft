#ifndef PLUGININTERFACE_TAGSLINEEDIT_H
#define PLUGININTERFACE_TAGSLINEEDIT_H
#include <memory>
#include <QLineEdit>
#include "config.h"
#include "categoryselector.h"

namespace LeechCraft
{
	namespace Util
	{
		class TagsCompleter;

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

			friend class TagsCompleter;

			std::auto_ptr<CategorySelector> CategorySelector_;
			TagsCompleter *Completer_;
		public:
			/** @brief Constructs the line edit widget.
			 *
			 * Creates the line edit widget.
			 *
			 * @param[in] parent Parent widget.
			 */
			PLUGININTERFACE_API TagsLineEdit (QWidget *parent);
			/** @brief Adds the selector widget to the line edit.
			 *
			 * Because this function uses the completion model, it should be
			 * used after the association with a TagsCompleter.
			 *
			 * @sa TagsCompleter
			 */
			PLUGININTERFACE_API void AddSelector ();
		public slots:
			/** @brief Completes the string.
			 *
			 * Completes the current text in line edit with completion passed
			 * throught string parameter.
			 *
			 * @param[in] string String with completion.
			 */
			PLUGININTERFACE_API void insertTag (const QString& string);
		private slots:
			void handleTagsUpdated (const QStringList&);
			void handleSelectionChanged (const QStringList&);
		protected:
			virtual void keyPressEvent (QKeyEvent*);
			virtual void focusInEvent (QFocusEvent*);
			virtual void contextMenuEvent (QContextMenuEvent*);
			void SetCompleter (TagsCompleter*);
		private:
			QString textUnderCursor () const;
		};
	};
};

#endif

