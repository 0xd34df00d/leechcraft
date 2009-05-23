#ifndef PLUGININTERFACE_TAGSCOMPLETER_H
#define PLUGININTERFACE_TAGSCOMPLETER_H
#include <QCompleter>
#include "config.h"

namespace LeechCraft
{
	class TagsManager;

	namespace Util
	{
		class TagsLineEdit;

		/** @brief Completer suitable for tag completion.
		 *
		 * Handles tag completions. One would need special class for this
		 * because standard QCompleter doesn't work: tag sequence isn't
		 * hierarchical, it is rather a set.
		 *
		 * @sa TagsCompletionModel
		 * @sa TagsLineEdit
		 */
		class TagsCompleter : public QCompleter
		{
			Q_OBJECT

			PLUGININTERFACE_API static QAbstractItemModel *CompletionModel_;
			friend class LeechCraft::TagsManager;
		public:
			/** @brief Constructs the completer.
			 *
			 * Sets up for completion and prepares line for work with itself.
			 *
			 * @param[in] line The line edit which would be used for tag
			 * completion.
			 * @param[in] parent Parent object.
			 */
			PLUGININTERFACE_API TagsCompleter (TagsLineEdit *line,
					QObject *parent = 0);

			/** @brief Path splitter override.
			 *
			 * Handles sequence of tags considering its set structure. Splits
			 * the path by spaces and returns the resulting string list.
			 *
			 * @param[in] path The tags sequence to split.
			 * @return Splitted sequence.
			 */
			PLUGININTERFACE_API virtual QStringList splitPath (const QString& path) const;
		protected:
			static void SetModel (QAbstractItemModel *model)
			{
				CompletionModel_ = model;
			}
		};
	};
};

#endif

