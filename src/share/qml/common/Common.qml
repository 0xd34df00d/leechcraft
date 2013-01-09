import QtQuick 1.1

QtObject {
    function getTooltipPos(item) {
        var absPoint = item.mapToItem(quarkDisplayRoot, item.x, item.y);
        absPoint = quarkProxy.mapToGlobal(absPoint.x, absPoint.y);
        absPoint.x += quarkDisplayRoot.width;
        return absPoint;
    }

    function showTooltip(item, func) {
        var absPoint = getTooltipPos(item);
        func(absPoint.x, absPoint.y);
    }
}
