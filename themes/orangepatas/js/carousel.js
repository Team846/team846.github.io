var timeout = 8000;
var speed = 500;
id = setTimeout(function(){Animate(2)}, timeout);
var amount;
var flag = 1;
var videoid = "l_TM2o045o8";
var width = 0;
var height = 0;
var playing = false;
function Animate(num)
{
	if (!playing)
	{
		if (document.getElementById(videoid))
		{
			if (!document.getElementById(videoid).hasOwnProperty("playerState"))
			{
				doAnimate(num);
			} else
			{
				if (document.getElementById(videoid).playerState() == "ended")
				{
					doAnimate(num);
				}
			}
		} else
		{
			doAnimate(num);
		}
	}
}

function clickAnimate(num)
{
	if (playing)
	{
		swfobject.removeSWF(videoid);
		document.getElementById("videopreview").removeChild(wrapper);
		$("#container").fadeOut(300);
		setTimeout(function() {
			$("#container").css("border", "0");
			$("#container").css("border-radius", "0");
			$("#container").css("box-shadow", "none");
			$("#container").css("padding", "0");
			$("#container").css("height", "265px");
			$("#container").css("width", "528px");
			$("#container").css("left", "423px");
			$("#container").css("top", "78px");
			$("#videopreview").css("background", "none");
			$("#videopreview").css("height", "265px");
			$("#videopreview").css("width", "528px");
			$("#videopreview").html('<img src="themes/orangepatas/img/play.png" id="play" width="150" style="position: absolute; top: 60px; left: 190px"/>');
			$("#container").fadeIn(0);
		}, 300);
		playing = false;
	}
	doAnimate(num);
}

function doAnimate(num)
{
	if(num != flag)
	{
		clearTimeout(id);
		$("#item" + flag).fadeOut(speed);
		$("#item" + num).fadeIn(speed);
		$("#button" + flag).removeClass("btnSelected");
		$("#button" + num).addClass("btnSelected");
		flag = num;
		if(flag == amount)
			id = setTimeout(function(){Animate(1)}, timeout);
		else
			id = setTimeout(function(){Animate(flag + 1)}, timeout);
	}
}

$(".homeMainScroll").ready(function() {
	amount = document.getElementsByClassName('banner')[0].getElementsByClassName('item').length;
	for(i = 1; i < amount; i++)
	{
		$("#item" + (i + 1)).hide();
	}
	$("#button1").addClass("btnSelected");
});

$("#videopreview").ready(function() {
	$("#videopreview").click(function() {
		if (!playing)
		{
			playing = true;
			var object = document.createElement("div");
			var wrapper = document.createElement("div");
			wrapper.id = "wrapper";
			object.id = videoid;
			width = $("#videopreview").width() + 1;
			height = width * 360.0 / 640.0;
			/*object.type = "application/x-shockwave-flash";
			object.width = width;
			object.height = height;
			object.style.position = "absolute";
			object.style.zIndex = "999";
			object.src = "themes/orangepatas/vid/youtube.swf";
			object.setAttribute("classid", "clsid:d27cdb6e-ae6d-11cf-96b8-444553540000");
			object.setAttribute("codebase", "http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,40,0");
			object.innerHTML = "<param name=\"movie\" value=\"themes/orangepatas/vid/youtube.swf\" /><param name=FlashVars value=\"id=" + id + "\" />";
			var embed = document.createElement("embed");
			embed.width = width;
			embed.height = height;
			embed.src = "themes/orangepatas/vid/youtube.swf";
			embed.setAttribute("flashvars", "id=" + id);
			object.appendChild(embed);*/
			wrapper.appendChild(object);
			document.getElementById("videopreview").innerHTML = "";
			document.getElementById("videopreview").appendChild(wrapper);
			$("#container").hide();
			$("#container").css("border", "1px solid #cecec8");
			$("#container").css("border-radius", "2px");
			$("#container").css("box-shadow", "0px 0px 4px 0px #eee");
			$("#container").css("padding", "2px");
			$("#container").css("height", height);
			$("#container").css("width", width);
			$("#container").css("left", "419px");
			$("#container").css("top", "74px");
			$("#videopreview").css("background", "black");
			$("#videopreview").css("height", height);
			$("#videopreview").css("width", width);
			var closebutton = document.createElement("div");
			closebutton.id = "close";
			document.getElementById("videopreview").appendChild(closebutton);
			$("#close").bind("click", function(e){ closePlayer(); e.stopPropagation() });
			$("#container").fadeIn(300);
			wrapper.style.opacity = 0;
			var flash = {};
			flash.id = videoid;
			var params = {};
			params.wmode = "opaque";
			if (swfobject.hasFlashPlayerVersion("9")) {
				//swfobject.registerObject(id, "11");
				swfobject.embedSWF("themes/orangepatas/vid/youtube.swf", videoid, width, height, "11", '', flash, params);
			} else
			{
				var iframe = document.createElement("iframe");
				iframe.src = "//www.youtube.com/embed/" + videoid;
				iframe.width = width;
				iframe.height = height;
				iframe.style.padding = 0;
				iframe.style.border = 0;
				object.appendChild(iframe);
				$("#wrapper").css("opacity", 1);
			}
		}
	});
});

// Called by youtube.swf when loaded
function onSwfLoaded()
{
	// Calls begin function in youtube.swf when loaded
	document.getElementById(videoid).begin();
	setTimeout(function() {
		$("#wrapper").css("opacity", 1);
	}, 50);
	return true;
}

function closePlayer()
{
	if (swfobject.hasFlashPlayerVersion("9")) {
		swfobject.removeSWF(videoid);
	}
	document.getElementById("videopreview").innerHTML = "";
	$("#container").fadeOut(300);
	setTimeout(function() {
		$("#container").css("border", "0");
		$("#container").css("border-radius", "0");
		$("#container").css("box-shadow", "none");
		$("#container").css("padding", "0");
		$("#container").css("height", "265px");
		$("#container").css("width", "528px");
		$("#container").css("left", "423px");
		$("#container").css("top", "78px");
		$("#videopreview").css("background", "none");
		$("#videopreview").css("height", "265px");
		$("#videopreview").css("width", "528px");
		$("#videopreview").html('<img src="themes/orangepatas/img/play.png" id="play" width="150" style="position: absolute; top: 60px; left: 190px"/>');
		$("#container").fadeIn(0);
	}, 300);
	playing = false;

	clearTimeout(id);
	id = setTimeout(function() {
		Animate(flag + 1);
	}, 2000);
}
