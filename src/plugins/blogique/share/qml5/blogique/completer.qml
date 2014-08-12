import QtQuick 2.3

Rectangle {
	id: completerRootRect
	border.width: 1

	property variant tagModel;

	height: 30 * Math.min(4, tagsModel.count)

	signal currentItemSelected(string tag);
	signal returnFocus();
	signal closeInput();

	onActiveFocusChanged:
		if (activeFocus)
			completer.forceActiveFocus()

	ListView {
		id: completer

		anchors.fill: parent

		clip: true
		boundsBehavior: Flickable.StopAtBounds;
		model: tagModel
		highlight: Rectangle {
			width: parent.width - 1;
			x: parent.x + 1;
			color: "#DCDCDC";
		}

		currentIndex: activeFocus ? currentIndex : -1;

		delegate: Component {
			id: completerDelegate

			Item {
				id: wrapper;
				width:parent.width;
				height: 30
				anchors.left: parent.left;
				anchors.leftMargin: 5

				Text {
					text: display;
				}

				function selectItem(index) {
					currentItemSelected (tagsModel.getTagName(index));
				}

				MouseArea {
					id: itemMouseArea
					anchors.fill: parent;
					hoverEnabled: true;

					onEntered: completer.currentIndex = index;
					onClicked: selectItem (index);
				}
			}
		}

		Keys.onUpPressed: {
			if (!currentIndex)
				returnFocus();
			else
				--currentIndex;
		}

		Keys.onDownPressed: {
			if (currentIndex == tagsModel.count)
				return;
			else
				++currentIndex;
		}

		Keys.onReturnPressed: currentItemSelected(tagsModel.GetTagName(currentIndex));

		Keys.onEscapePressed: closeInput();
	}

	Connections {
		target: tagsModel
		onCountChanged: parent.height =  30 * Math.min(4, tagsModel.count);
	}
}
