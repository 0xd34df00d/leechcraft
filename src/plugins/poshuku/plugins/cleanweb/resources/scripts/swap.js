function swapFlashObjects() {
    var src = "%1";
    var type = "application/x-shockwave-flash";
    var objects = arguments[0];
    for (var i = 0; i < objects.length; ++i) {
        if (type != objects[i].getAttribute("type"))
            continue;
        var newObject = objects[i].cloneNode(true);
        newObject.setAttribute("type", "application/futuresplash");
        var parent = objects[i].parentNode;
        parent.replaceChild(newObject, objects[i]);
    }
}

swapFlashObjects(document.getElementsByTagName("object"));
swapFlashObjects(document.getElementsByTagName("embed"));
