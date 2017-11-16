/* JavaScript to run on every page load */
$(document).ready(function() {
	$("a").has("img").css('background', 'none');
	$("a").has("img").css('padding', '0');
	if (swfobject.hasFlashPlayerVersion("9")) {
		var iframes = document.getElementsByTagName("iframe");
		for (var i = iframes.length - 1; i >= 0; i--)
		{
			if (iframes[i].src.indexOf("youtube.com") != -1)
			{
				var reg = new RegExp('(?:https?://)?(?:www\\.)?(?:youtu\\.be/|youtube\\.com(?:/embed/|/v/|/watch\\?v=))([\\w-]{10,12})', 'g');
				var matches = reg.exec(iframes[i].src);
				var wrapper = document.createElement("div");
				var object = document.createElement("div");
				object.id = matches[1];
				object.type = "application/x-shockwave-flash";
				object.height = iframes[i].height;
				object.width = iframes[i].width;
				wrapper.style.cssText = iframes[i].style.cssText;
				/*object.src = "themes/orangepatas/vid/youtube.swf";
				object.setAttribute("classid", "clsid:d27cdb6e-ae6d-11cf-96b8-444553540000");
				object.setAttribute("codebase", "http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,40,0");
				object.innerHTML = "<param name=\"movie\" value=\"themes/orangepatas/vid/youtube.swf\" /><param name=FlashVars value=\"id=" + matches[1] + "\" />";
				var embed = document.createElement("embed");
				embed.height = iframes[i].height;
				embed.width = iframes[i].width;
				embed.src = "themes/orangepatas/vid/youtube.swf";
				embed.setAttribute("flashvars", "id=" + matches[1]);
				object.appendChild(embed);*/
				var flash = {};
				flash.id = matches[1];
				//swfobject.registerObject(matches[1], "11");
				wrapper.appendChild(object);
				iframes[i].parentNode.replaceChild(wrapper, iframes[i]);
				swfobject.embedSWF("themes/orangepatas/vid/youtube.swf", matches[1], object.width, object.height, "11", '', flash);
			}
		}
	}
});