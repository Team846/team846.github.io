package 
{
	import flash.display.MovieClip;
	import flash.events.Event;
	import flash.events.MouseEvent;
	import fl.events.SliderEvent;
	import fl.events.*;
	import flash.text.TextFormat;
	import flash.external.*;
	import flash.utils.Timer;

	import YoutubePlayer;
	import fl.controls.Slider;

	public class Main extends MovieClip
	{
		private var youtubeplayer:YoutubePlayer;
		private var lastLocation = 0;
		private var dragging = false;
		private var vol = 100;
		private var muted = false;
		private var started = false;
		public function Main()
		{
			ExternalInterface.addCallback("playerState", playerState);
			ExternalInterface.addCallback("playPausePlayer", playPausePlayer);
			ExternalInterface.addCallback("begin", externalBegin);
		}

		private function checkForJavascript():void{
			var readyTimer:Timer = new Timer(50, 0);
			readyTimer.addEventListener(TimerEvent.TIMER, timerHandler);
			readyTimer.start();
		}
		private function timerHandler(evt:TimerEvent):void {
			var isReady:Boolean = jsReadyState();
			if (isReady) Timer(evt.target).stop();
		}
		private function jsReadyState():Boolean{
			var readyState:Boolean = ExternalInterface.call("onSwfLoaded");
			return readyState;
		}

		private function init(e)
		{
			removeEventListener(Event.ENTER_FRAME, init);//removes the previous listener
			youtubeplayer = new YoutubePlayer(id);//instaniates the player with this video id
			nav.master.vol_ctrl.visible = false;
			nav.play_mc.gotoAndStop(2);
			nav.play_mc.buttonMode = nav.master.volume_mc.buttonMode = true;
			nav.play_mc.addEventListener(MouseEvent.CLICK, playPausePlayer);
			nav.master.volume_mc.addEventListener(MouseEvent.CLICK, muteUnmutePlayer);
			nav.bar_mc.addEventListener(MouseEvent.MOUSE_DOWN, scrub);
			stage.addEventListener(KeyboardEvent.KEY_DOWN, key);
			nav.master.vol_ctrl.volume_slider.addEventListener(Event.CHANGE, changeVol);
			nav.master.addEventListener(MouseEvent.ROLL_OVER, showControl);
			nav.master.addEventListener(MouseEvent.ROLL_OUT, hideControl);
			for (var i:Number = 0; i < numChildren; i++)
			{
				trace(getChildAt(i).name);
			}
			started = false;
			holder_mc.addChild(youtubeplayer);
			nav.fullness_mc.width = 0;
			nav.progress_mc.width = 0;
			addEventListener(Event.ENTER_FRAME, loop);
		}
		private function begin(e)
		{
			start_mc.removeEventListener(MouseEvent.CLICK, begin);
			gotoAndStop(2);
			setChildIndex(holder_mc, 1);
			setChildIndex(nav, 2);
			setChildIndex(frame_mc, 0);
			lastLocation = 0;
			addEventListener(Event.ENTER_FRAME, init);
			frame_mc.addEventListener(MouseEvent.ROLL_OVER, mMove);
			stage.addEventListener(Event.MOUSE_LEAVE, leave);
		}

		private function externalBegin():void
		{
			start_mc.removeEventListener(MouseEvent.CLICK, begin);
			gotoAndStop(2);
			setChildIndex(holder_mc, 1);
			setChildIndex(nav, 2);
			setChildIndex(frame_mc, 0);
			lastLocation = 0;
			addEventListener(Event.ENTER_FRAME, init);
			frame_mc.addEventListener(MouseEvent.ROLL_OVER, mMove);
			stage.addEventListener(Event.MOUSE_LEAVE, leave);
		}

		private function showControl(e)
		{
			nav.master.vol_ctrl.visible = true;
		}

		private function hideControl(e)
		{
			nav.master.vol_ctrl.volume_slider.stopDrag();
			nav.master.vol_ctrl.visible = false;
		}

		private function changeVol(e)
		{
			youtubeplayer.setVolume(nav.master.vol_ctrl.volume_slider.value);
		}

		private function key(e)
		{
			if (e.keyCode == 32)
			{
				if (youtubeplayer.playerStatus == "playing")
				{
					youtubeplayer.pauseVideo();
					nav.play_mc.gotoAndStop(1);
				}
				else
				{
					youtubeplayer.playVideo();
					nav.play_mc.gotoAndStop(2);
				}
			}
		}

		private function playPausePlayer(e)
		{
			if (youtubeplayer.playerStatus == "playing")
			{
				youtubeplayer.pauseVideo();
				nav.play_mc.gotoAndStop(1);
			}
			else
			{
				youtubeplayer.playVideo();
				nav.play_mc.gotoAndStop(2);
			}
		}

		private function muteUnmutePlayer(e)
		{
			if (youtubeplayer.ismuted)
			{
				youtubeplayer.unmuteVideo();
			}
			else
			{
				youtubeplayer.muteVideo();
			}
		}

		private function stopScrub(e)
		{
			if (dragging)
			{
				setChildIndex(holder_mc, 1);
				setChildIndex(nav, 2);
				setChildIndex(frame_mc, 0);
				dragging = false;
				var distance:Number = (e.stageX - nav.scrubber_mc.width / 2) - nav.fullness_mc.x;
				var ratio:Number = distance / 510;
				if (ratio > 1)
				{
					ratio = 1;
				}
				youtubeplayer.seekTo(ratio * youtubeplayer.getDuration(), true);
				lastLocation = ratio;
				stage.removeEventListener(MouseEvent.MOUSE_UP, stopScrub);
				youtubeplayer.removeEventListener(MouseEvent.MOUSE_UP, stopScrub);
			}
		}

		private function scrub(e)
		{
			setChildIndex(holder_mc, 0);
			setChildIndex(nav, 2);
			setChildIndex(frame_mc, 1);
			dragging = true;
			if ((e.stageX - nav.scrubber_mc.width / 2 >= nav.fullness_mc.x) && (e.stageX - nav.scrubber_mc.width / 2 <= nav.fullness_mc.x + 510))
			{
				nav.scrubber_mc.x = e.stageX - nav.scrubber_mc.width / 2;
			}
			else if (e.stageX - nav.scrubber_mc.width / 2 < nav.fullness_mc.x)
			{
				nav.scrubber_mc.x = nav.fullness_mc.x;
			}
			else if (e.stageX - nav.scrubber_mc.width / 2 > nav.fullness_mc.x + 510)
			{
				nav.scrubber_mc.x = nav.fullness_mc.x + 510;
			}
			stage.addEventListener(MouseEvent.MOUSE_UP, stopScrub);
			youtubeplayer.addEventListener(MouseEvent.MOUSE_UP, stopScrub);
		}

		private function loop(e)
		{
			if (youtubeplayer != null)
			{
				if (youtubeplayer.playerStatus == "ended")
				{
					ExternalInterface.call("closePlayer");
					youtubeplayer.destroy();
					holder_mc.removeChild(youtubeplayer);
					youtubeplayer = null;
					this.removeEventListener(Event.ENTER_FRAME, fadein);
					this.removeEventListener(Event.ENTER_FRAME, fadeout);
					stage.removeEventListener(Event.MOUSE_LEAVE, leave);
					stage.removeEventListener(MouseEvent.MOUSE_UP, stopScrub);
					nav.master.vol_ctrl.volume_slider.removeEventListener(Event.CHANGE, changeVol);
					nav.master.volume_mc.removeEventListener(MouseEvent.ROLL_OVER, showControl);
					nav.master.volume_mc.removeEventListener(MouseEvent.ROLL_OUT, hideControl);
					nav.master.volume_mc.removeEventListener(MouseEvent.CLICK, muteUnmutePlayer);
					for (var i:Number = 0; i < numChildren; i++)
					{
						removeChild(getChildAt(i));
					}
					gotoAndStop(3);
					return;
				}
				if (! started)
				{
					if (muted)
					{
						youtubeplayer.muteVideo();
					}
					else
					{
						youtubeplayer.setVolume(vol);
					}
					started = true;
				}
				if (dragging)
				{
					if ((mouseX - nav.scrubber_mc.width / 2 >= nav.fullness_mc.x) && (mouseX - nav.scrubber_mc.width / 2 <= nav.fullness_mc.x + 510))
					{
						nav.scrubber_mc.x = mouseX - nav.scrubber_mc.width / 2;
					}
					else if (mouseX - nav.scrubber_mc.width / 2 < nav.fullness_mc.x)
					{
						nav.scrubber_mc.x = nav.fullness_mc.x;
					}
					else if (mouseX - nav.scrubber_mc.width / 2 > nav.fullness_mc.x + 510)
					{
						nav.scrubber_mc.x = nav.fullness_mc.x + 510;
					}
				}
				else
				{
					nav.scrubber_mc.x = nav.fullness_mc.x + nav.fullness_mc.width;
				}
				nav.fullness_mc.width = youtubeplayer.fullnessRatio * 510;
				nav.progress_mc.width = (lastLocation * 510) + youtubeplayer.progressRatio * 510;
				if (nav.progress_mc.width > 510)
				{
					nav.progress_mc.width = 510;
				}
				nav.master.vol_ctrl.volume_slider.value = youtubeplayer.getVolume();
				if (youtubeplayer.getVolume() == 0)
				{
					nav.master.volume_mc.gotoAndStop(2);
				}
				else
				{
					nav.master.volume_mc.gotoAndStop(1);
				}
				muted = youtubeplayer.ismuted;
				vol = nav.master.vol_ctrl.volume_slider.value;
				var componentFormat:TextFormat = new TextFormat();
				componentFormat.size = 14;//change the font size
				componentFormat.font = "Trebuchet MS";//font type
				componentFormat.bold = true;
				nav.time_text.setTextFormat(componentFormat);
				nav.time_text.text = formatTime(youtubeplayer.getCurrentTime()) + "/" + formatTime(youtubeplayer.getDuration());
			}
		}

		private function playerState():String
		{
			if (youtubeplayer != null)
				return youtubeplayer.playerStatus;
			else
				return "ended";
		}

		private static function formatTime(time:Number, detailLevel:uint = 2):String
		{
			var HOURS:uint = 2;
			var MINUTES:uint = 1;
			var SECONDS:uint = 0;
			var intTime:uint = Math.floor(time);
			var hours:uint = Math.floor(intTime/ 3600);
			var minutes:uint = (intTime - (hours * 3600)) / 60;
			var seconds:uint = intTime - (hours * 3600) - (minutes * 60);
			var hourString:String = detailLevel == HOURS ? hours + ":":"";
			var minuteString:String = detailLevel >= MINUTES ? ((detailLevel == HOURS && minutes <10 ? "0":"") + minutes + ":"):"";
			var secondString:String = ((seconds < 10 && (detailLevel >= MINUTES)) ? "0":"") + seconds;
			if (hourString == '0:')
			{
				return minuteString + secondString;
			}
			else
			{
				return hourString + minuteString + secondString;
			}
		}
	}
}