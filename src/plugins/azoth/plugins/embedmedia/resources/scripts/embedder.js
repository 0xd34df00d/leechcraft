///{{{ parseUri
// parseUri 1.2.2
// (c) Steven Levithan <stevenlevithan.com>
// MIT License

function parseUri (str) {
	var	o   = parseUri.options,
		m   = o.parser[o.strictMode ? "strict" : "loose"].exec(str),
		uri = {},
		i   = 14;

	while (i--) uri[o.key[i]] = m[i] || "";

	uri[o.q.name] = {};
	uri[o.key[12]].replace(o.q.parser, function ($0, $1, $2) {
		if ($1) uri[o.q.name][$1] = $2;
	});

	return uri;
};

parseUri.options = {
	strictMode: false,
	key: ["source","protocol","authority","userInfo","user","password","host","port","relative","path","directory","file","query","anchor"],
	q:   {
		name:   "queryKey",
		parser: /(?:^|&)([^&=]*)=?([^&]*)/g
	},
	parser: {
		strict: /^(?:([^:\/?#]+):)?(?:\/\/((?:(([^:@]*)(?::([^:@]*))?)?@)?([^:\/?#]*)(?::(\d*))?))?((((?:[^?#\/]*\/)*)([^?#]*))(?:\?([^#]*))?(?:#(.*))?)/,
		loose:  /^(?:(?![^:@]+:[^:@\/]*@)([^:\/?#.]+):)?(?:\/\/)?((?:(([^:@]*)(?::([^:@]*))?)?@)?([^:\/?#]*)(?::(\d*))?)(((\/(?:[^?#](?![^?#\/]*\.[^?#\/.]+(?:[?#]|$)))*\/?)?([^?#\/]*))(?:\?([^#]*))?(?:#(.*))?)/
	}
};
///}}}

String.prototype.fmt = function () {
    var txt = this;
    for (var i = 0; i < arguments.length; i += 1) {
        var exp = new RegExp('\\{' + (i) + '\\}', 'gm');
        txt = txt.replace(exp, arguments[i]);
    }
    return txt;
};

String.prototype.contains = function(substr){
	return this.indexOf(substr) != -1;
}

function getYoutubeEmbed(url){
	var vid = parseUri(url).queryKey.v;

	return  '<object width="480" height="360"> \
	  		 <param name="movie" value="http://www.youtube.com/v/{0}&amp;autoplay=1"></param> \
			 <param name="wmode" value="transparent"></param> \
			 <embed src="http://www.youtube.com/v/{1}&amp;autoplay=1" \
			 type="application/x-shockwave-flash" wmode="transparent" width="480" height="360"> \
	  		 </embed></object>'.fmt(vid, vid);
}

function getGoogleVideEmbed(url){
	var vid = parseUri(url).queryKey.docid;
	
	return '<embed id="VideoPlayback" \
			  src="http://video.google.com/googleplayer.swf?docid={0}" \
			  style="width:480px;height:360px" \
			  allowFullScreen="true" \
			  allowScriptAccess="always" \
			  type="application/x-shockwave-flash"> \
			  </embed>'.fmt(vid);
}

function getVimeoEmbed(url){
	var vid = parseUri(url).path.substr(1);
	
	return  '<object width="480" height="360">\
			  <param name="allowfullscreen" value="true" />\
			  <param name="allowscriptaccess" value="always" />\
			  <param name="movie" value="http://vimeo.com/moogaloop.swf?clip_id={0}&amp;server=vimeo.com&amp;show_title=1&amp;show_byline=1&amp;show_portrait=0&amp;color=&amp;fullscreen=1" />\
			  <embed src="http://vimeo.com/moogaloop.swf?clip_id={0}&amp;server=vimeo.com&amp;show_title=1&amp;show_byline=1&amp;show_portrait=0&amp;color=&amp;fullscreen=1" type="application/x-shockwave-flash" allowfullscreen="true" allowscriptaccess="always" width="480" height="360">\
			  </embed>\
			  </object>'.fmt(vid, vid);
}

function getRutubeEmbed(url){
	var vid = parseUri(url).queryKey.v;

	return '<object width="480" height="360">\
			  <param name="movie" value="http://video.rutube.ru/{0}"></param>\
			  <param name="wmode" value="window"></param>\
			  <param name="allowFullScreen" value="true"></param>\
			  <embed src="http://video.rutube.ru/{1}" type="application/x-shockwave-flash" wmode="window" width="480" height="360" allowFullScreen="true" >\
			  </embed>\
			  </object>'.fmt(vid, vid);
}

function getMetacafeEmbed(url){
	var video_url = url.replace("watch", "fplayer");
	var last_char_index = video_url.length - 1;
	
	if (video_url[last_char_index] === '/')
		video_url = video_url.substr(0, last_char_index)

	return '<embed src="{0}.swf" width="480" height="360" \
			  wmode="transparent" \
			  pluginspage="http://www.macromedia.com/go/getflashplayer" \
			  type="application/x-shockwave-flash" \
			  allowFullScreen="true" \
			  allowScriptAccess="always"> </embed>'.fmt(video_url);
}

function getTwitVidEmbed(url){
    var vid = parseUri(url).path.slice(1);

    return '<iframe width="480" height="360" \
        frameborder="0" src="http://www.twitvid.com/embed.php?guid={0}">'.fmt(vid);
}

function getMp3PlayerEmbed(url) {
    return '<object align="middle" width="400px" height="27px"> \
        <param name="allowScriptAccess" value="never"> \
        <param name="allowFullScreen" value="true"> \
        <param name="wmode" value="transparent"> \
        <param name="movie" value="http://www.google.com/reader/ui/3523697345-audio-player.swf?audioUrl={0}"> \
        <embed classname="audio-player-embed" \
            width="400px" \
            height="27px" \
            type="application/x-shockwave-flash" \
            src="http://www.google.com/reader/ui/3523697345-audio-player.swf?audioUrl={0}"></object>'.fmt(encodeURI(url));
}

function getEmbed(url){
	var service = parseUri(url).host;

	if(service.contains("youtube.com")){
		return getYoutubeEmbed(url);
	} else if (service.contains("vimeo.com")){
		return getVimeoEmbed(url);
	} else if (service.contains("rutube.ru")){
		return getRutubeEmbed(url);
	} else if (service.contains("metacafe.com")){
		return getMetacafeEmbed(url);
	} else if (service.contains("video.google.com")){
		return getGoogleVideEmbed(url);
	} else if (service.contains("twitvid.com")){
        return getTwitVidEmbed(url);
    } else if (url.match(/\.mp3$/i)) {
        return getMp3PlayerEmbed(url);
    }
	return null;
}

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

function resize(width, height) {
    return function(e) {
        var embed = this.parentNode.parentNode.firstChild.getElementsByTagName("embed");

        if (embed && embed.length) {
            embed[0].setAttribute('width', width);
            embed[0].setAttribute('height', height);
        }
        this.parentNode.parentNode.firstChild.setAttribute('width', width);
        this.parentNode.parentNode.firstChild.setAttribute('height', height);
        this.parentNode.parentNode.firstChild.setAttribute('style', 'width: {0}px;height: {1}px;'.fmt(width, height));
        this.parentNode.parentNode.setAttribute('style', 'width: {0}px;'.fmt(width));
        e.preventDefault();
        return false;
    }
}

function addResizer(div, size, width, height) {
    var newA;

    newA = document.createElement('a');
    newA.setAttribute('href', '#');
    newA.appendChild(document.createTextNode(size));
    addEvent(newA, 'click', resize(width, height));
    div.appendChild(newA);
    div.appendChild(document.createTextNode(' | '));
}

function clickOnVideoLink(e) {
    var embedCode = getEmbed(this.href);

	if (embedCode) {
		var playerMainDiv = document.createElement('div');
		var newA;
		var newDiv;

		playerMainDiv.innerHTML = embedCode;

		playerMainDiv.setAttribute('style', 'width: 480px;display:block');
		newDiv = document.createElement('p');
		newDiv.setAttribute('style', 'float: left;');
		newDiv.appendChild(document.createTextNode(' video size : '));

		if (!this.href.match(/\.mp3$/i)) {
            addResizer(newDiv, 'normal', '480', '360');
            addResizer(newDiv, 'big', '640', '480');
            addResizer(newDiv, 'huge', '800', '600');
            playerMainDiv.appendChild(newDiv);
        }

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
        else if (allLinksArray[i].href.search(/rutube.ru/i) > - 1) {
            addEvent(allLinksArray[i], 'click', clickOnVideoLink);
        }
        else if (allLinksArray[i].href.search(/metacafe.com/i) > - 1) {
            addEvent(allLinksArray[i], 'click', clickOnVideoLink);
        }
        else if (allLinksArray[i].href.search(/video.google.com/i) > - 1) {
            addEvent(allLinksArray[i], 'click', clickOnVideoLink);
        }
        else if (allLinksArray[i].href.match(/\.mp3$/i))
        {
            addEvent(allLinksArray[i], 'click', clickOnVideoLink);
        }
    }
},
false);
