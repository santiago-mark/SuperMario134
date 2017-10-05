struct Coin{
	bool taken;
	int x;
	int y;
};

/********************************************************Animation Definitions***********************************************************/
/*animation definitions of piranha plant*/
AnimDef coin_AnimDef{
	"coin",
	{ { 0, .1 }, { 1, .1 }, { 2, .1 }, { 1, .1 } },
	20,
	4
};
/**************************************************************Animation Data************************************************************/
/*animation of piranha plant*/
AnimData coin_AnimData = {
	&coin_AnimDef,
	0,
	0.1,
	true
};

/*********************************************************Mapping*******************************************************************************/
Coin coins_in_map[] = {
	{false, 17,23},
	{ false, 18, 23 },
	{ false, 27, 21 },
	{ false, 28, 20 },
	{ false, 29, 19 },
};
