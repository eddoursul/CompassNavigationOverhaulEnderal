class CompassMarkerInfo extends MovieClip
{
	static var UNITS_TO_METERS:Number = 0.01428;
	static var UNITS_TO_FEET:Number = 0.046875;

	// Instances
	var HeightIndicatorInstance:MovieClip;
	var Distance:MovieClip;
	var Target:MovieClip;

	// References
	var Movie:MovieClip;

	// Variables
	var Index:Number;
	var Objective:String;
	var Location:String;
	var UseMetricUnits:Boolean;

	public function CompassMarkerInfo()
	{
		Index = -1;
		Distance.TextFieldInstance.autoSize = "center";
	}

	public function SetDistanceAndHeightDifference(a_distance:Number, a_heightDifference:Number):Void
	{
		if (UseMetricUnits)
		{
			Distance.TextFieldInstance.text = String(Math.floor(a_distance * UNITS_TO_METERS)) + " m";
		}
		else
		{
			Distance.TextFieldInstance.text = String(Math.floor(a_distance * UNITS_TO_FEET)) + " ft ";
		}

		Distance.HeightIndicatorInstance._x = Distance.TextFieldInstance._x + Distance.TextFieldInstance._width +
											  Distance.HeightIndicatorInstance._width / 3;

		// Height like 3 or 4 floors. Measured in Skyrim units.
		if (a_heightDifference > 840)
		{
			Distance.HeightIndicatorInstance._alpha = Math.max(Movie._alpha, 75);
			Distance.HeightIndicatorInstance.gotoAndStop("Above");
		}
		else if (a_heightDifference < -840)
		{
			Distance.HeightIndicatorInstance._alpha = Math.max(Movie._alpha, 75);
			Distance.HeightIndicatorInstance.gotoAndStop("Below");
		}
		else
		{
			Distance.HeightIndicatorInstance._alpha = 0;
		}
	}
}
