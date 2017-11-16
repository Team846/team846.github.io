function countdownTimer(string) {
	now      = new Date();
	kickoff  = Date.parse(string);
	diff = kickoff - now;
	if (diff <= 0)
	{
		document.getElementById("countdown").innerHTML = 'Kickoff Is Now!';
		document.getElementById("until").innerHTML = '';
		return;
	}
	days  = Math.floor( diff / (1000*60*60*24) );
	hours = Math.floor( diff / (1000*60*60) );
	mins  = Math.floor( diff / (1000*60) );
	secs  = Math.floor( diff / 1000 );

	dd = days;
	hh = hours - days  * 24;
	mm = mins  - hours * 60;
	ss = secs  - mins  * 60;
	
	document.getElementById("countdown").innerHTML = dd + '<span style="font-size: 8pt">' + ((dd == 1) ? 'DAY ' : 'DAYS') + '</span>' + ((String(hh).length < 2) ? String("0" + hh) : String(hh)) + '<span style="font-size: 8pt">HRS </span>' + ((String(mm).length < 2) ? String("0" + mm) : String(mm)) + '<span style="font-size: 8pt">MIN </span>' + ((String(ss).length < 2) ? String("0" + ss) : String(ss)) + '<span style="font-size: 8pt">SEC</span>';
	document.getElementById("until").innerHTML = 'Until Championships!';
}