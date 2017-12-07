
unsigned char SpeedFromElevation(char elevation);



unsigned char SpeedFromElevation(char elevation)
{
	static struct {
		char elevation;	//vertical camera pwm when locked
		unsigned char speed; //corresponding ball launcher speed for best shot
	} table[]= {	//data for best shot, ordered low angle to high angle (far to close)
		{-127,0},	//first entry
		{-127,0},
		{-127,0},
		{ 127,0}		//last entry
	};

	for (i=sizeof(table)/sizeof(table[0]; i>0; i--)
	{
		int deltaElevation = elevation - table[i-1].elevation;
		if (deltaElevation < 0)
			continue;
	
		//interpolate between table[i-1] and table[i]
		speed = deltaElevation * (table[i].speed - table[i-1].speed);
		speed /= (table[i-1].elevation - table[i].elevation;
	}
	return speed;

}
