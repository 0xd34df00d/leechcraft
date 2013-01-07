import QtQuick 1.1

QtObject {
    function getAbsPos(field, item) {
        var result = 0;
        while (item)
        {
            result += item[field];
            item = item.parent;
        }
        return result;
    }

    function showTooltip(item, func) {
        var absPoint = quarkProxy.mapToGlobal(getAbsPos("x", item), getAbsPos("y", item));
        func(absPoint.x + quarkDisplayRoot.width, absPoint.y);
    }
}
