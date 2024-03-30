
class QuestItem extends MovieClip
{
	// Static variables
	static public var mainQuestFrame:Number;
	static public var magesGuildQuestFrame:Number;
	static public var thievesGuildQuestFrame:Number;
	static public var darkBrotherhoodQuestFrame:Number;
	static public var companionQuestFrame:Number;
	static public var miscQuestFrame:Number;
	static public var daedricQuestFrame:Number;
	static public var favorQuestFrame:Number;
	static public var civilWarQuestFrame:Number;
	static public var dlc01QuestFrame:Number;
	static public var dlc02QuestFrame:Number;
	static private var bracketOpenLongestNameFrame:Number;

	// Instances
	var Title:MovieClip;
	var ObjectiveItemList:Array;

	// References
	private var TitleTextField:TextField;
	public var TitleEndPiece:MovieClip;
	private var TitleBracket:MovieClip;

	// Variables
	public var isBeingShown:Boolean;
	public var ageIndex:Number;

	public function QuestItem()
	{
		ObjectiveItemList = new Array();

		TitleTextField = Title.TitleTextField;
		TitleEndPiece = Title.EndPiece;
		TitleBracket = Title.Bracket;

		TitleTextField.textAutoSize = "shrink";

		TitleEndPiece.gotoAndStop("Main");
		mainQuestFrame = TitleEndPiece._currentframe;

		TitleEndPiece.gotoAndStop("MagesGuild");
		magesGuildQuestFrame = TitleEndPiece._currentframe;

		TitleEndPiece.gotoAndStop("ThievesGuild");
		thievesGuildQuestFrame = TitleEndPiece._currentframe;

		TitleEndPiece.gotoAndStop("DarkBrotherhood");
		darkBrotherhoodQuestFrame = TitleEndPiece._currentframe;

		TitleEndPiece.gotoAndStop("Companion");
		companionQuestFrame = TitleEndPiece._currentframe;

		TitleEndPiece.gotoAndStop("Misc");
		miscQuestFrame = TitleEndPiece._currentframe;

		TitleEndPiece.gotoAndStop("Daedric");
		daedricQuestFrame = TitleEndPiece._currentframe;

		TitleEndPiece.gotoAndStop("Favor");
		favorQuestFrame = TitleEndPiece._currentframe;

		TitleEndPiece.gotoAndStop("CivilWar");
		civilWarQuestFrame = TitleEndPiece._currentframe;

		TitleEndPiece.gotoAndStop("DLC01");
		dlc01QuestFrame = TitleEndPiece._currentframe;

		TitleEndPiece.gotoAndStop("DLC02");
		dlc02QuestFrame = TitleEndPiece._currentframe;

		TitleBracket.gotoAndStop("longest name");
		bracketOpenLongestNameFrame = TitleBracket._currentframe;
	}

	public function SetQuestInfo(a_type:Number, a_title:String, a_isInSameLocation:Boolean, a_objectives:Array, a_ageIndex:Number):Void
	{
		// Set end piece art
		TitleEndPiece.gotoAndStop(a_type);

		// Set quest title as in the journal
		TitleTextField.text = a_title.toUpperCase();

		// Make room for the title
		var bracketOpenFrame:Number = Math.min(bracketOpenLongestNameFrame, Math.ceil(TitleTextField.textWidth * bracketOpenLongestNameFrame / TitleTextField._width));
		TitleBracket.gotoAndStop(bracketOpenFrame);

		var yOffset = TitleBracket._y + TitleBracket._height;
		for (var i:Number = 0; i < a_objectives.length; i++)
		{
			var objectiveItem:MovieClip = attachMovie("ObjectiveItem", "objective" + i, getNextHighestDepth(), { _x:Title._x, _y: yOffset });

			if (a_isInSameLocation)
			{
				objectiveItem.Objective.gotoAndStop("NormalSelected");
			}
			objectiveItem.Objective.textField.text = a_objectives[i];

			ObjectiveItemList.push(objectiveItem);

			yOffset += 1.1 * objectiveItem._height;
		}

		ageIndex = a_ageIndex;
	}



	public function SetSide(a_side:String):Void
	{
		TitleEndPiece.SideArt.gotoAndStop(a_side);
	}

	public function Show():Void
	{
		gotoAndPlay("FadeIn");

		for (var i:Number = 0; i < ObjectiveItemList.length; i++)
		{
			ObjectiveItemList[i].gotoAndPlay("FadeIn");
		}
	}

	public function Remove():Void
	{
		for (var i:Number = 0; i < ObjectiveItemList.length; i++)
		{
			ObjectiveItemList[i].gotoAndPlay("FadeOut");
		}

		gotoAndPlay("FadeOut");
	}
}
