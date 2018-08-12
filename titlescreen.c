#include "titlescreen.h"

#include <prism/blitz.h>

#include "worldscreen.h"

static struct {
	MugenSpriteFile mSprites;

} gData;

static void loadTitleScreen() {
	gData.mSprites = loadMugenSpriteFileWithoutPalette("title/TITLE.sff");
	addMugenAnimation(createOneFrameMugenAnimationForSprite(1, 0), &gData.mSprites, makePosition(0, 0, 1));
	addFadeIn(30, NULL, NULL);
}


static void gotoWorldScreen(void* tCaller) {
	(void)tCaller;
	resetDays();
	loadCountries();
	setNewScreen(&WorldScreen);
}

static void updateTitleScreen() {

	if (hasPressedAFlank() || hasPressedStartFlank()) {
		addFadeOut(30, gotoWorldScreen, NULL);
	}
}

Screen TitleScreen = {
	.mLoad = loadTitleScreen,
	.mUpdate = updateTitleScreen,
};