package 
{
	import flash.display.MovieClip;
	import flash.display.Loader;
	import flash.events.Event;
	import flash.system.Security;
	import flash.net.URLRequest;

	public class YoutubePlayer extends MovieClip
	{

		var player:Object;//the object wich will have the player loaded to
		var loader:Loader;//the loader wich will load the player
		var id:String;//the video's id
		var playerStatus:String;//returns the players current playing status
		var progressRatio:Number;//returns the ratio difference between the bytes loaded and the bytes total, from 0 to 1, (usefull for the progress bar)
		var fullnessRatio:Number;//returns the ratio difference between the playhead and the total seconds, from 0 to 1, (usefull for the fullness bar)
		var ismuted:Boolean;// returns true if player is muted

		//when instanced we need the video's id passed to it
		public function YoutubePlayer($id:String)
		{
			Security.allowDomain("www.youtube.com");
			//allow access from youtube;
			id = $id;//sets the id
			loader = new Loader();//instanciates the loader
			loader.contentLoaderInfo.addEventListener(Event.INIT, onLoaderInit);
			//After loading, calls onLoaderInit;
			loader.load(new URLRequest("http://www.youtube.com/apiplayer?version=3"));
		}
		//starts loading process;

		private function onLoaderInit(event:Event):void
		{
			addChild(loader);//adds the loader to stage 
			loader.content.addEventListener("onReady", onPlayerReady);
			//called when the player is ready;
			loader.content.addEventListener("onError", onPlayerError);
			//called when the player has errors;
			loader.content.addEventListener("onStateChange", onPlayerStateChange);
		}

		private function onPlayerReady(event:Event):void
		{
			player = loader.content;
			player.setSize(640, 360);
			player.loadVideoById(id);
			player.setPlaybackQuality("hd720");
			addEventListener(Event.ENTER_FRAME, updatePlayer);
		}

		private function onPlayerError(event:Event):void
		{
			trace("player error:", Object(event).data);
		}

		private function onPlayerStateChange(event:Event):void
		{
			trace("player state:", Object(event).data);
		}

		//Wrappers for outside controlling
		public function playVideo()
		{
			player.playVideo();
		}

		public function pauseVideo()
		{
			player.pauseVideo();
		}

		public function stopVideo()
		{
			player.stopVideo();
		}

		public function muteVideo()
		{
			player.mute();
		}

		public function unmuteVideo()
		{
			player.unMute();
		}

		public function changeQuality(quality:String):void
		{
			player.setPlaybackQuality(quality);
		}

		public function getCurrentTime():Number
		{
			return player.getCurrentTime();
		}

		public function getDuration():Number
		{
			return player.getDuration();
		}

		public function seekTo(time:Number, buffer:Boolean):void
		{
			player.seekTo(time, buffer);
		}
		
		public function setVolume(vol:Number)
		{
			player.setVolume(vol);
		}
		
		public function getVolume():Number
		{
			return player.getVolume();
		}
		
		public function destroy()
		{
			removeEventListener(Event.ENTER_FRAME, updatePlayer);
			player.destroy();
			player = null;
			removeChild(loader);
			loader = null;
		}

		public function updatePlayer(e)
		{

			ismuted = player.isMuted();//returns true if muted

			//sets the progress ratio
			progressRatio = player.getVideoBytesLoaded() / player.getVideoBytesTotal();

			//sets the fullness ratio
			fullnessRatio = player.getCurrentTime() / player.getDuration();

			//sets the playerStatus for outside use
			switch (player.getPlayerState())
			{
				case -1 :
					playerStatus = "unstarted";
					break;
				case 0 :
					playerStatus = "ended";
					break;
				case 1 :
					playerStatus = "playing";
					break;
				case 2 :
					playerStatus = "paused";
					break;
			}
		}
	}
}