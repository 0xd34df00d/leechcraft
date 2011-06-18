function isNumber(n) {
	return ! isNaN(parseFloat(n)) && isFinite(n);
}

function addEvent(obj, sEvent, sFunc) {
	if (obj.addEventListener) obj.addEventListener(sEvent, sFunc, false);
	else if (obj.attachEvent) obj.attachEvent('on' + sEvent, sFunc);
}

function insertAfter(referenceNode, node) {
	referenceNode.parentNode.insertBefore(node, referenceNode.nextSibling);
}

function clickOnVideoLink(e) {
	var videoId;
	if ((this.href.search(/youtube.com/i) > - 1) && (this.href.search(/v=/i) > - 1)) {
		videoId = this.href.slice(this.href.search(/v=/i) + 2);
		if (videoId.indexOf('&') > 1) {
			videoId = videoId.slice(0, videoId.indexOf('&'));
		}
		videoId = 'http://www.youtube.com/embed/' + videoId + '?autoplay=1&fs=1';
	}
	else if (this.href.search(/vimeo.com/i) > - 1) {
		videoId = this.href.slice(this.href.search(/vimeo.com/i) + 10);
		if (isNumber(videoId)) {
			videoId = 'http://player.vimeo.com/video/' + videoId + '?autoplay=1';
		}
	}
	else if (this.href.search(/twitvid.com/i) > - 1) {
		videoId = 'http://www.twitvid.com/embed.php?guid=' + this.href.slice(this.href.search(/twitvid.com/i) + 12);
	}

	if (videoId.length > 0) {
		var playerMainDiv = document.createElement('div');
		var newA;
		var newDiv;
		var iframeObj = document.createElement('iframe');
		iframeObj.setAttribute('width', '480');
		iframeObj.setAttribute('height', '360');
		iframeObj.setAttribute('frameborder', '0');
		iframeObj.setAttribute('src', videoId);
		playerMainDiv.appendChild(iframeObj);

		playerMainDiv.setAttribute('style', 'width: 480px;');
		newDiv = document.createElement('p');
		newDiv.setAttribute('style', 'float: left;');
		newDiv.appendChild(document.createTextNode(' video size : '));

		newA = document.createElement('a');
		newA.setAttribute('href', '#');
		newA.appendChild(document.createTextNode('normal'));
		addEvent(newA, 'click', function(e) {
			this.parentNode.parentNode.firstChild.setAttribute('width', '480');
			this.parentNode.parentNode.setAttribute('style', 'width: 480px;');
			this.parentNode.parentNode.firstChild.setAttribute('height', '360');
			e.preventDefault();
			return false;
		});
		newDiv.appendChild(newA);
		newDiv.appendChild(document.createTextNode(' | '));

		newA = document.createElement('a');
		newA.setAttribute('href', '#');
		newA.appendChild(document.createTextNode('big'));
		addEvent(newA, 'click', function(e) {
			this.parentNode.parentNode.firstChild.setAttribute('width', '640');
			this.parentNode.parentNode.setAttribute('style', 'width: 640px;');
			this.parentNode.parentNode.firstChild.setAttribute('height', '480');
			e.preventDefault();
			return false;
		});
		newDiv.appendChild(newA);
		newDiv.appendChild(document.createTextNode(' | '));

		newA = document.createElement('a');
		newA.setAttribute('href', '#');
		newA.appendChild(document.createTextNode('huge'));
		addEvent(newA, 'click', function(e) {
			this.parentNode.parentNode.firstChild.setAttribute('width', '800');
			this.parentNode.parentNode.setAttribute('style', 'width: 800px;');
			this.parentNode.parentNode.firstChild.setAttribute('height', '600');
			e.preventDefault();
			return false;
		});
		newDiv.appendChild(newA);
		playerMainDiv.appendChild(newDiv);

		newDiv = document.createElement('p');
		newDiv.setAttribute('style', 'float: right;');
		newA = document.createElement('a');
		newA.setAttribute('href', '#');
		newA.appendChild(document.createTextNode('close player'));
		addEvent(newA, 'click', function(e) {
			this.parentNode.parentNode.previousSibling.setAttribute('style', this.parentNode.parentNode.previousSibling.getAttribute('bkpstyle'));
			this.parentNode.parentNode.parentNode.removeChild(this.parentNode.parentNode);
			e.preventDefault();
			return false;
		});
		newDiv.appendChild(newA);
		playerMainDiv.appendChild(newDiv);
		newDiv = document.createElement('br');
		newDiv.setAttribute('clear', 'all');
		playerMainDiv.appendChild(newDiv);

		insertAfter(this, playerMainDiv);
		this.setAttribute('bkpstyle', this.getAttribute('style'));
		this.setAttribute('style', 'display: none');

		e.preventDefault();
		return false;
	}
}

document.addEventListener("DOMNodeInserted", function(evt) {
    console.log("DOMNodeInserted");
    var allLinksArray = evt.target.getElementsByTagName('a');

    for (var i = 0; i < allLinksArray.length; i++) {
        if (allLinksArray[i].href.search(/youtube.com/i) > - 1) {
            addEvent(allLinksArray[i], 'click', clickOnVideoLink);
        }
        else if (allLinksArray[i].href.search(/vimeo.com/i) > - 1) {
            addEvent(allLinksArray[i], 'click', clickOnVideoLink);
        }
        else if (allLinksArray[i].href.search(/twitvid.com/i) > - 1) {
            addEvent(allLinksArray[i], 'click', clickOnVideoLink);
        }
    }
},
false);
console.log("Yahoo");
