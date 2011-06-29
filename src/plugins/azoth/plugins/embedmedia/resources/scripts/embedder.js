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

function addStyle(css) {
    var head = document.getElementsByTagName('head')[0];
    if (head) {
        var style = document.createElement("style");
        style.type = "text/css";
        style.appendChild(document.createTextNode(css));
        head.appendChild(style);
    }
}

function isImageLink(link) {
    return  link.match(/.jpeg$/i) ||
            link.match(/.jpg$/i) ||
            link.match(/.png$/i) ||
            link.match(/.gif$/) ||
            link.match(/pics.livejournal.com/) ||
            link.match(/img.leprosorium.com/)
}



function getYoutubeEmbed(url) {
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

/// {{{ Lightbox

var loadingImage = 'data:image/gif;base64,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';		
var closeButton = 'data:image/gif;base64,R0lGODlhFAAUAJEDAJeXl0BAQO7u7vr69SH5BAEAAAMALAAAAAAUABQAAAI+nI+py+0PYxO02puutrmCcAUA1VHBWZ2gUJqoWrUuHCMbXR+aiuZG+OqxbCkhrfUJjYa6zUbmJEGdiygHUQAAOw==';		





//
// getPageScroll()
// Returns array with x,y page scroll values.
// Core code from - quirksmode.org
//
function getPageScroll(){

	var yScroll;

	if (self.pageYOffset) {
		yScroll = self.pageYOffset;
	} else if (document.documentElement && document.documentElement.scrollTop){	 // Explorer 6 Strict
		yScroll = document.documentElement.scrollTop;
	} else if (document.body) {// all other Explorers
		yScroll = document.body.scrollTop;
	}

	arrayPageScroll = new Array('',yScroll) 
	return arrayPageScroll;
}



//
// getPageSize()
// Returns array with page width, height and window width, height
// Core code from - quirksmode.org
// Edit for Firefox by pHaez
//
function getPageSize(){
	
	var xScroll, yScroll;
	
	if (window.innerHeight && window.scrollMaxY) {	
		xScroll = document.body.scrollWidth;
		yScroll = window.innerHeight + window.scrollMaxY;
	} else if (document.body.scrollHeight > document.body.offsetHeight){ // all but Explorer Mac
		xScroll = document.body.scrollWidth;
		yScroll = document.body.scrollHeight;
	} else { // Explorer Mac...would also work in Explorer 6 Strict, Mozilla and Safari
		xScroll = document.body.offsetWidth;
		yScroll = document.body.offsetHeight;
	}
	
	var windowWidth, windowHeight;
	if (self.innerHeight) {	// all except Explorer
		windowWidth = self.innerWidth;
		windowHeight = self.innerHeight;
	} else if (document.documentElement && document.documentElement.clientHeight) { // Explorer 6 Strict Mode
		windowWidth = document.documentElement.clientWidth;
		windowHeight = document.documentElement.clientHeight;
	} else if (document.body) { // other Explorers
		windowWidth = document.body.clientWidth;
		windowHeight = document.body.clientHeight;
	}	
	
	// for small pages with total height less then height of the viewport
	if(yScroll < windowHeight){
		pageHeight = windowHeight;
	} else { 
		pageHeight = yScroll;
	}

	// for small pages with total width less then width of the viewport
	if(xScroll < windowWidth){	
		pageWidth = windowWidth;
	} else {
		pageWidth = xScroll;
	}


	arrayPageSize = new Array(pageWidth,pageHeight,windowWidth,windowHeight) 
	return arrayPageSize;
}


//
// pause(numberMillis)
// Pauses code execution for specified time. Uses busy code, not good.
// Code from http://www.faqts.com/knowledge_base/view.phtml/aid/1602
//
function pause(numberMillis) {
	var now = new Date();
	var exitTime = now.getTime() + numberMillis;
	while (true) {
		now = new Date();
		if (now.getTime() > exitTime)
			return;
	}
}

//
// getKey(key)
// Gets keycode. If 'x' is pressed then it hides the lightbox.
//

function getKey(e){
	if (e == null) { // ie
		keycode = event.keyCode;
	} else { // mozilla
		keycode = e.which;
	}
	key = String.fromCharCode(keycode).toLowerCase();
	
	if(key == 'x'){ hideLightbox(); }
}


//
// listenKey()
//
function listenKey () {	document.onkeypress = getKey; }
	

//
// showLightbox()
// Preloads images. Pleaces new image in lightbox then centers and displays.
//
function showLightbox(objLink)
{
	// prep objects
	var objOverlay = document.getElementById('overlay');
	var objLightbox = document.getElementById('lightbox');
	var objCaption = document.getElementById('lightboxCaption');
	var objImage = document.getElementById('lightboxImage');
	var objLoadingImage = document.getElementById('loadingImage');
	var objLightboxDetails = document.getElementById('lightboxDetails');

	
	var arrayPageSize = getPageSize();
	var arrayPageScroll = getPageScroll();

	// center loadingImage if it exists
	if (objLoadingImage) {
		objLoadingImage.style.top = (arrayPageScroll[1] + ((arrayPageSize[3] - 35 - objLoadingImage.height) / 2) + 'px');
		objLoadingImage.style.left = (((arrayPageSize[0] - 20 - objLoadingImage.width) / 2) + 'px');
		objLoadingImage.style.display = 'block';
	}

	// set height of Overlay to take up whole page and show
	objOverlay.style.height = (arrayPageSize[1] + 'px');
	objOverlay.style.display = 'block';

	// preload image
	imgPreload = new Image();

	imgPreload.onload=function(){
		objImage.src = objLink.href;

		// center lightbox and make sure that the top and left values are not negative
		// and the image placed outside the viewport
		var lightboxTop = arrayPageScroll[1] + ((arrayPageSize[3] - 35 - imgPreload.height) / 2);
		var lightboxLeft = ((arrayPageSize[0] - 20 - imgPreload.width) / 2);
		
		objLightbox.style.top = (lightboxTop < 0) ? "0px" : lightboxTop + "px";
		objLightbox.style.left = (lightboxLeft < 0) ? "0px" : lightboxLeft + "px";


		objLightboxDetails.style.width = imgPreload.width + 'px';
		
		if(objLink.getAttribute('title')){
			objCaption.style.display = 'block';
			//objCaption.style.width = imgPreload.width + 'px';
			objCaption.innerHTML = objLink.getAttribute('title');
		} else {
			objCaption.style.display = 'none';
		}
		
		// A small pause between the image loading and displaying is required with IE,
		// this prevents the previous image displaying for a short burst causing flicker.
		if (navigator.appVersion.indexOf("MSIE")!=-1){
			pause(250);
		} 

		if (objLoadingImage) {	objLoadingImage.style.display = 'none'; }

		// Hide select boxes as they will 'peek' through the image in IE
		selects = document.getElementsByTagName("select");
        for (i = 0; i != selects.length; i++) {
                selects[i].style.visibility = "hidden";
        }

	
		objLightbox.style.display = 'block';

		// After image is loaded, update the overlay height as the new image might have
		// increased the overall page height.
		arrayPageSize = getPageSize();
		objOverlay.style.height = (arrayPageSize[1] + 'px');
		
		// Check for 'x' keypress
		listenKey();

		return false;
	}

	imgPreload.src = objLink.href;
	
}





//
// hideLightbox()
//
function hideLightbox()
{
	// get objects
	objOverlay = document.getElementById('overlay');
	objLightbox = document.getElementById('lightbox');

	// hide lightbox and overlay
	objOverlay.style.display = 'none';
	objLightbox.style.display = 'none';

	// make select boxes visible
	selects = document.getElementsByTagName("select");
    for (i = 0; i != selects.length; i++) {
		selects[i].style.visibility = "visible";
	}

	// disable keypress listener
	document.onkeypress = '';
}




//
// initLightbox()
// Function runs on window load, going through link tags looking for rel="lightbox".
// These links receive onclick events that enable the lightbox display for their targets.
// The function also inserts html markup at the top of the page which will be used as a
// container for the overlay pattern and the inline image.
//
function initLightbox()
{
	// the rest of this code inserts html at the top of the page that looks like this:
	//
	// <div id="overlay">
	//		<a href="#" onclick="hideLightbox(); return false;"><img id="loadingImage" /></a>
	//	</div>
	// <div id="lightbox">
	//		<a href="#" onclick="hideLightbox(); return false;" title="Click anywhere to close image">
	//			<img id="closeButton" />		
	//			<img id="lightboxImage" />
	//		</a>
	//		<div id="lightboxDetails">
	//			<div id="lightboxCaption"></div>
	//			<div id="keyboardMsg"></div>
	//		</div>
	// </div>
	
	var objBody = document.getElementsByTagName("body").item(0);
	
	// create overlay div and hardcode some functional styles (aesthetic styles are in CSS file)
	var objOverlay = document.createElement("div");
	objOverlay.setAttribute('id','overlay');
	objOverlay.onclick = function () {hideLightbox(); return false;}
	objOverlay.style.display = 'none';
	objOverlay.style.position = 'absolute';
	objOverlay.style.top = '0';
	objOverlay.style.left = '0';
	objOverlay.style.zIndex = '90';
 	objOverlay.style.width = '100%';
	objBody.insertBefore(objOverlay, objBody.firstChild);
	
	var arrayPageSize = getPageSize();
	var arrayPageScroll = getPageScroll();

	// preload and create loader image
	var imgPreloader = new Image();
	
	// if loader image found, create link to hide lightbox and create loadingimage
	imgPreloader.onload=function(){

		var objLoadingImageLink = document.createElement("a");
		objLoadingImageLink.setAttribute('href','#');
		objLoadingImageLink.onclick = function () {hideLightbox(); return false;}
		objOverlay.appendChild(objLoadingImageLink);
		
		var objLoadingImage = document.createElement("img");
		objLoadingImage.src = loadingImage;
		objLoadingImage.setAttribute('id','loadingImage');
		objLoadingImage.style.position = 'absolute';
		objLoadingImage.style.zIndex = '150';
		objLoadingImageLink.appendChild(objLoadingImage);

		imgPreloader.onload=function(){};	//	clear onLoad, as IE will flip out w/animated gifs

		return false;
	}

	imgPreloader.src = loadingImage;

	// create lightbox div, same note about styles as above
	var objLightbox = document.createElement("div");
	objLightbox.setAttribute('id','lightbox');
	objLightbox.style.display = 'none';
	objLightbox.style.position = 'absolute';
	objLightbox.style.zIndex = '100';	
	objBody.insertBefore(objLightbox, objOverlay.nextSibling);
	
	// create link
	var objLink = document.createElement("a");
	objLink.setAttribute('href','#');
	objLink.setAttribute('title','Click to close');
	objLink.onclick = function () {hideLightbox(); return false;}
	objLightbox.appendChild(objLink);

	// preload and create close button image
	var imgPreloadCloseButton = new Image();

	// if close button image found, 
	imgPreloadCloseButton.onload=function(){

		var objCloseButton = document.createElement("img");
		objCloseButton.src = closeButton;
		objCloseButton.setAttribute('id','closeButton');
		objCloseButton.style.position = 'absolute';
		objCloseButton.style.zIndex = '200';
		objLink.appendChild(objCloseButton);

		return false;
	}

	imgPreloadCloseButton.src = closeButton;

	// create image
	var objImage = document.createElement("img");
	objImage.setAttribute('id','lightboxImage');
	objLink.appendChild(objImage);
	
	// create details div, a container for the caption and keyboard message
	var objLightboxDetails = document.createElement("div");
	objLightboxDetails.setAttribute('id','lightboxDetails');
	objLightbox.appendChild(objLightboxDetails);

	// create caption
	var objCaption = document.createElement("div");
	objCaption.setAttribute('id','lightboxCaption');
	objCaption.style.display = 'none';
	objLightboxDetails.appendChild(objCaption);

	// create keyboard message
	var objKeyboardMsg = document.createElement("div");
	objKeyboardMsg.setAttribute('id','keyboardMsg');
	objKeyboardMsg.innerHTML = 'press <a href="#" onclick="hideLightbox(); return false;"><kbd>x</kbd></a> to close';
	objLightboxDetails.appendChild(objKeyboardMsg);
}

/// }}} Lightbox


addStyle(
"#lightbox{" + 
"	background-color:#eee;" + 
"	padding: 10px;" + 
"	border-bottom: 2px solid #666;" + 
"	border-right: 2px solid #666;" + 
"	}" + 
"#lightboxDetails{" + 
"	font-size: 0.8em;" + 
"	padding-top: 0.4em;" + 
"	}	" + 
"#lightboxCaption{ float: left; }" + 
"#keyboardMsg{ float: right; }" + 
"#closeButton{ top: 5px; right: 5px; }" + 
"#lightbox img{ border: none; clear: both;} " + 
"#overlay img{ border: none; }" + 
"#overlay{background-image: url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAGUAAABlCAYAAABUfC3PAAAABGdBTUEAAK/INwWK6QAAABl0RVh0U29mdHdhcmUAQWRvYmUgSW1hZ2VSZWFkeXHJZTwAAACpSURBVHja7NEBDQAACMOwg39PWMMGCZ2EtZJMdKq2AIqgQBEUKIICRVAEBYqgQBEUKIIiKFAEBYqgQBEUQYEiKFAEBYqgCAoUQYEiKFAERVCgCAoUQYEiKIICRVCgCAoUQREUKIICRVCgCIqgQBEUKIICRVAEBYqgQBEUKIIiKFAEBYqgQBEUQYEiKFAEBYqgQLEAiqBAERQoggJFUAQFiqBAEZTHrQADAOi7AYkbZwBkAAAAAElFTkSuQmCC);}"
);
initLightbox();

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
        else if (isImageLink(allLinksArray[i].href))
        {
            allLinksArray[i].onclick = function () {showLightbox(this); return false;}
        }
    }
},
false);
