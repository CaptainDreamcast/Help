#include "worldscreen.h"

#include <prism/blitz.h>

#include "storyscreen.h"

typedef struct {
	char mName[100];
	char mDesc[1000];
	char mShort[100];
	int mHasUsedCustom[3];

	int mPopulation;
	int mGrowth;
} Country;

typedef struct {
	int mStage;

	int mBGEntity;
	int mCountryText;
	int mCountryName;

	int mPopulationText;
	int mPopulation;

	int mGrowthText;
	int mGrowth;

	int mDescText;
	int mDesc;

	int mSelect[3];
	int mSelectText[3];

	int mCurrentDisplayedCountry;
	int mRealCurrentCountry;

	int mCurrentSelection;
	int mIsSelectActive;
	int mIsSelectFinished;
	int mIsPickingWar;
	int mIsFadingOut;

	int mWarFirst;
	int mWarSecond;

	double mSelectDX;
} DescriptionMenu;

typedef struct {
	int mIDs[1200][1024];

} WorldMap;

static struct {
	int mDay;
	
	int mCountryAmount;
	Country mCountries[20];

	MugenSpriteFile mSprites;
	MugenAnimations mAnimations;
	MugenSounds mSounds;

	int mBackgroundEntity;
	int mBackgroundEntity2;
	int mCrosshairEntity;
	Position mCrosshairPosition;

	DescriptionMenu mDescription;
	WorldMap mMap;

	int mHasLost;
	int mHasWon;
} gData;

static void loadSingleCountry(MugenDefScriptGroup* tGroup) {
	int id = gData.mCountryAmount;

	char* str = getAllocatedMugenDefStringOrDefaultAsGroup(tGroup, "name", "NO_COUNTRY");
	strcpy(gData.mCountries[id].mName, str);
	freeMemory(str);

	str = getAllocatedMugenDefStringOrDefaultAsGroup(tGroup, "short", "NO");
	strcpy(gData.mCountries[id].mShort, str);
	freeMemory(str);

	char* str1 = getAllocatedMugenDefStringOrDefaultAsGroup(tGroup, "desc1", "NO_DESC");
	char* str2 = getAllocatedMugenDefStringOrDefaultAsGroup(tGroup, "desc2", " ");
	char* str3 = getAllocatedMugenDefStringOrDefaultAsGroup(tGroup, "desc3", " ");
	strcpy(gData.mCountries[id].mDesc, str3);
	sprintf(gData.mCountries[id].mDesc, "%s %s %s", str1, str2, str3);
	freeMemory(str1);
	freeMemory(str2);
	freeMemory(str3);

	gData.mCountries[id].mPopulation = getMugenDefIntegerOrDefaultAsGroup(tGroup, "population", 100000000);
	gData.mCountries[id].mGrowth = getMugenDefIntegerOrDefaultAsGroup(tGroup, "growth", 3);

	int i;
	for (i = 0; i < 3; i++) {
		gData.mCountries[id].mHasUsedCustom[i] = 0;
	}

	gData.mCountryAmount++;
}

void loadCountries() {
	MugenDefScript script = loadMugenDefScript("world/COUNTRIES.def");

	gData.mCountryAmount = 0;
	MugenDefScriptGroup* current = script.mFirstGroup;
	while (current) {
		loadSingleCountry(current);
		current = current->mNext;
	}

	unloadMugenDefScript(script);
}

static void loadDescription() {
	gData.mDescription.mBGEntity = addBlitzEntity(makePosition(0 - 184, 0, 20));
	addBlitzMugenAnimationComponentStatic(gData.mDescription.mBGEntity, &gData.mSprites, 3000, 1);

	gData.mDescription.mCountryText = addMugenText("REGION:", makePosition(10 - 184, 20, 21), 1);
	gData.mDescription.mCountryName = addMugenText("lmao", makePosition(10 - 184, 30, 21), 1);

	gData.mDescription.mPopulationText = addMugenText("POPULATION:", makePosition(10 - 184, 50, 21), 1);
	gData.mDescription.mPopulation = addMugenText("1.000.000.000", makePosition(10 - 184, 60, 21), 1);

	gData.mDescription.mGrowthText = addMugenText("GROWTH:", makePosition(10 - 184, 80, 21), 1);
	gData.mDescription.mGrowth = addMugenText("3%", makePosition(10 - 184, 90, 21), 1);


	gData.mDescription.mDescText = addMugenText("DESCRIPTION:", makePosition(10 - 184, 110, 21), 1);
	gData.mDescription.mDesc = addMugenText("ehhh", makePosition(10 - 184, 120, 21), 1);
	setMugenTextTextBoxWidth(gData.mDescription.mDesc, 164);

	gData.mDescription.mSelect[0] = addBlitzEntity(makePosition(92 - 184, 282, 21));
	addBlitzMugenAnimationComponent(gData.mDescription.mSelect[0], &gData.mSprites, &gData.mAnimations, 3000);
	gData.mDescription.mSelect[1] = addBlitzEntity(makePosition(92 - 184, 359, 21));
	addBlitzMugenAnimationComponent(gData.mDescription.mSelect[1], &gData.mSprites, &gData.mAnimations, 3000);
	gData.mDescription.mSelect[2] = addBlitzEntity(makePosition(92 - 184, 436, 21));
	addBlitzMugenAnimationComponent(gData.mDescription.mSelect[2], &gData.mSprites, &gData.mAnimations, 3000);

	gData.mDescription.mSelectText[0] = addMugenTextMugenStyle("\"NATURAL\" DISASTER", makePosition(92 - 184, 286, 22), makeVector3DI(1, 0, 0));
	gData.mDescription.mSelectText[1] = addMugenTextMugenStyle("IMPROVE EDUCATION", makePosition(92 - 184, 363, 22), makeVector3DI(1, 0, 0));
	gData.mDescription.mSelectText[2] = addMugenTextMugenStyle("START A WAR", makePosition(92 - 184, 440, 22), makeVector3DI(1, 0, 0));


	gData.mDescription.mCurrentDisplayedCountry = -1;
	gData.mDescription.mStage = 0;

	gData.mDescription.mCurrentSelection = 0;
	gData.mDescription.mIsSelectActive = 0;
	gData.mDescription.mIsSelectFinished = 0;
	gData.mDescription.mIsPickingWar = 0;
	gData.mDescription.mIsFadingOut = 0;
	gData.mDescription.mSelectDX = 0.01;
}

static void addDotsToNumber(char* tDst, uint64_t tSrc) {
	char temp[100];

	sprintf(temp, "%llu", tSrc);
	int n = strlen(temp);
	int dotAmount = n > 1 ? (n - 1) / 3 : 0;

	tDst[n + dotAmount] = '\0';
	int i = 0;
	int s = n - 1;
	for (n = n + dotAmount - 1; n >= 0; n--) {
		if (++i == 4) {
			i = 0;
			tDst[n] = '.';
		}
		else if (s >= 0) {
			tDst[n] = temp[s];
			s--;
		}
		else {
			tDst[n] = '\0';
		}
	}
}


static uint64_t getWorldPopulation() {
	uint64_t ret = 0;
	int i;
	for (i = 0; i < gData.mCountryAmount; i++) {
		ret += gData.mCountries[i].mPopulation;
	}

	return ret;
}

static void loadWorldPopulation() {
	int id;
	id = addMugenText("WORLD POPULATION:", makePosition(410, 20, 110), 1);

	uint64_t population = getWorldPopulation();

	if (population > 11000000000LL) {
		gData.mHasLost = 1;
	} else if (population <= 1000000000LL) {
		gData.mHasWon = 1;
	}

	char buffer[100];
	addDotsToNumber(buffer, population);
	id = addMugenText(buffer, makePosition(410, 30, 110), 1);

}

static void winCB(void* tCaller) {
	setCurrentStoryDefinitionFile("story/INTRO/OUTRO.def", 3);
	setNewScreen(&StoryScreen);
}

static void gotoWin() {
	addFadeOut(30, winCB, NULL);
}

static void loseCB(void* tCaller) {
	setCurrentStoryDefinitionFile("story/INTRO/OVER.def", 3);
	setNewScreen(&StoryScreen);
}

static void gotoLose() {
	addFadeOut(30, loseCB, NULL);
}

static void updateCountryPopulations() {
	if (!gData.mDay) return;

	int i;
	for (i = 0; i < gData.mCountryAmount; i++) {
		gData.mCountries[i].mPopulation = max(0, (int)(gData.mCountries[i].mPopulation * (1 + (gData.mCountries[i].mGrowth / 100.0))));

		if (gData.mCountries[i].mPopulation > 2000000000) {
			gData.mHasLost = 1;
		}
	}



}

static void loadWorldHitbox() {
	Buffer buf = fileToBuffer("world/HITBOX.bmp");
	uint8_t* src = buf.mData;

	int i;
	int n = 122 + 1024 * 1106 * 3;
	int y = 1106-1, x = 0;
	for (i = 122; i < n; ) {
		uint8_t b = src[i];
		uint8_t g = src[i + 1];
		uint8_t r = src[i + 2];

		int id;
		if (r == 128 && g == 0 && b == 255) id = 0;
		else if (r == 0 && g == 255 && b == 128) id = 1;
		else if (r == 0 && g == 0 && b == 255) id = 2;
		else if (r == 255 && g == 255 && b == 0) id = 3;
		else if (r == 255 && g == 0 && b == 0) id = 4;
		else if (r == 255 && g == 128 && b == 0) id = 5;
		else if (r == 255 && g == 255 && b == 255) id = 6;
		else if (r == 0 && g == 128 && b == 255) id = 7;
		else if (r == 0 && g == 255 && b == 0) id = 8;
		else if (r == 0 && g == 0 && b == 0) id = 9;
		else id = -1;

		//if(id != -1 && id != 9) printf("%d %d %d\n", x, y, id);

		gData.mMap.mIDs[y][x] = id;

		x++;
		if (x >= 1024) {
			x = 0;
			y--;
		}

		i += 3;
	}


	freeBuffer(buf);

}

static void loadWorldScreen() {
	gData.mHasLost = 0;
	gData.mHasWon = 0;

	loadWorldHitbox();
	gData.mSprites = loadMugenSpriteFileWithoutPalette("world/WORLD.sff");
	gData.mAnimations = loadMugenAnimationFile("world/WORLD.air");
	gData.mSounds = loadMugenSoundFile("world/WORLD.snd");

	gData.mBackgroundEntity = addBlitzEntity(makePosition(0, 0, 1));
	addBlitzMugenAnimationComponentStatic(gData.mBackgroundEntity, &gData.mSprites, 1000, 0);
	gData.mBackgroundEntity2 = addBlitzEntity(makePosition(1028, 0, 1));
	addBlitzMugenAnimationComponentStatic(gData.mBackgroundEntity2, &gData.mSprites, 1000, 0);

	gData.mCrosshairPosition = makePosition(430, 240, 10);
	gData.mCrosshairEntity = addBlitzEntity(gData.mCrosshairPosition);
	addBlitzMugenAnimationComponentStatic(gData.mCrosshairEntity, &gData.mSprites, 2000, 0);

	int id = addBlitzEntity(makePosition(640, 0, 10));
	addBlitzMugenAnimationComponentStatic(id, &gData.mSprites, 3000, 0);

	updateCountryPopulations();
	loadDescription();
	loadWorldPopulation();

	playTrack(4);

	gData.mDay++;
}

static void updateWorldMovement() {
	if (!gData.mDescription.mIsPickingWar && gData.mDescription.mIsSelectActive) return;
	if (gData.mDescription.mIsFadingOut) return;
	
	double speed = 2;

	Position delta = makePosition(0, 0, 0);
	if (hasPressedLeft()) {
		addBlitzEntityPositionX(gData.mBackgroundEntity, speed);
		delta.x -= speed;
	}
	else if (hasPressedRight()) {
		addBlitzEntityPositionX(gData.mBackgroundEntity, -speed);
		delta.x += speed;
	}

	if (hasPressedUp()) {
		addBlitzEntityPositionY(gData.mBackgroundEntity, speed);
		delta.y -= speed;
	}
	else if (hasPressedDown()) {
		addBlitzEntityPositionY(gData.mBackgroundEntity, -speed);
		delta.y += speed;
	}

	double py = getBlitzEntityPositionY(gData.mBackgroundEntity);
	if (py > 0) {
		delta.y += py - 0;
		setBlitzEntityPositionY(gData.mBackgroundEntity, 0);
	}
	if (py < -(1104 - 480)) {
		delta.y -= -(1104 - 480) - py;
		setBlitzEntityPositionY(gData.mBackgroundEntity, -(1104 - 480));
	}

	gData.mCrosshairPosition = vecAdd(gData.mCrosshairPosition, delta);

	if (gData.mCrosshairPosition.x < 0) {
		double px = getBlitzEntityPositionX(gData.mBackgroundEntity);
		setBlitzEntityPositionX(gData.mBackgroundEntity, px - 1024);
		gData.mCrosshairPosition.x += 1024;
	}
	if (gData.mCrosshairPosition.x >= 1024) {
		double px = getBlitzEntityPositionX(gData.mBackgroundEntity);
		setBlitzEntityPositionX(gData.mBackgroundEntity, px + 1024);
		gData.mCrosshairPosition.x -= 1024;
	}

	double px = getBlitzEntityPositionX(gData.mBackgroundEntity);
	if (px > 0) {
		setBlitzEntityPositionX(gData.mBackgroundEntity2, px - 1024);
		setBlitzEntityPositionY(gData.mBackgroundEntity2, py);
	}
	if (px < -(1024 - 640)) {
		setBlitzEntityPositionX(gData.mBackgroundEntity2, px + 1024);
		setBlitzEntityPositionY(gData.mBackgroundEntity2, py);
	}


}

static void updateCrosshairRotation() {
	addBlitzEntityRotationZ(gData.mCrosshairEntity, 0.05);
}

static void updateDescriptionCountry(int tCurrent) {
	changeMugenText(gData.mDescription.mCountryName, gData.mCountries[tCurrent].mName);
	changeMugenText(gData.mDescription.mDesc, gData.mCountries[tCurrent].mDesc);
	setMugenTextBuildup(gData.mDescription.mDesc, 1);

	char value[20];
	addDotsToNumber(value, gData.mCountries[tCurrent].mPopulation);
	changeMugenText(gData.mDescription.mPopulation, value);

	sprintf(value, "%d%%", gData.mCountries[tCurrent].mGrowth);
	changeMugenText(gData.mDescription.mGrowth, value);
}

static void setSelectInactive();

static void updateDescriptionActive() {
	int current = gData.mMap.mIDs[(int)gData.mCrosshairPosition.y][(int)gData.mCrosshairPosition.x];
	//printf("%d %d %d\n", (int)gData.mCrosshairPosition.x, (int)gData.mCrosshairPosition.y, current);

	if (current == -1) {
		if (!gData.mDescription.mIsPickingWar) {
			gData.mDescription.mStage = 0;
			if (gData.mDescription.mIsSelectActive) setSelectInactive();
			gData.mDescription.mIsSelectFinished = 0;
			gData.mDescription.mCurrentDisplayedCountry = -1;
		}
	}
	else {
		gData.mDescription.mStage = 1;
		if (current != gData.mDescription.mCurrentDisplayedCountry) updateDescriptionCountry(current);
		gData.mDescription.mCurrentDisplayedCountry = current;
	}

	gData.mDescription.mRealCurrentCountry = current;
}

static void updateDescriptionPosition() {
	double speed = 10;

	double dx = 0;
	if (gData.mDescription.mStage == 0) {
		dx = -10;
		addBlitzEntityPositionX(gData.mDescription.mBGEntity, -speed);	
		double nx = getBlitzEntityPositionX(gData.mDescription.mBGEntity);
		if (nx < -184) {
			dx += -184 - nx;		
		}
	}
	else if (gData.mDescription.mStage == 1) {
		dx = 10;
		addBlitzEntityPositionX(gData.mDescription.mBGEntity, speed);
		double nx = getBlitzEntityPositionX(gData.mDescription.mBGEntity);
		if (nx > 0) {
			dx -= nx - 0;
		}
	}

	setBlitzEntityPositionX(gData.mDescription.mBGEntity, min(max(getBlitzEntityPositionX(gData.mDescription.mBGEntity), -184), 0));

	setMugenTextPosition(gData.mDescription.mCountryText, vecAdd(getMugenTextPosition(gData.mDescription.mCountryText), makePosition(dx, 0, 0)));
	setMugenTextPosition(gData.mDescription.mCountryName, vecAdd(getMugenTextPosition(gData.mDescription.mCountryName), makePosition(dx, 0, 0)));

	setMugenTextPosition(gData.mDescription.mPopulationText, vecAdd(getMugenTextPosition(gData.mDescription.mPopulationText), makePosition(dx, 0, 0)));
	setMugenTextPosition(gData.mDescription.mPopulation, vecAdd(getMugenTextPosition(gData.mDescription.mPopulation), makePosition(dx, 0, 0)));

	setMugenTextPosition(gData.mDescription.mGrowthText, vecAdd(getMugenTextPosition(gData.mDescription.mGrowthText), makePosition(dx, 0, 0)));
	setMugenTextPosition(gData.mDescription.mGrowth, vecAdd(getMugenTextPosition(gData.mDescription.mGrowth), makePosition(dx, 0, 0)));

	setMugenTextPosition(gData.mDescription.mDescText, vecAdd(getMugenTextPosition(gData.mDescription.mDescText), makePosition(dx, 0, 0)));
	setMugenTextPosition(gData.mDescription.mDesc, vecAdd(getMugenTextPosition(gData.mDescription.mDesc), makePosition(dx, 0, 0)));

	int i;
	for (i = 0; i < 3; i++) {
		setMugenTextPosition(gData.mDescription.mSelectText[i], vecAdd(getMugenTextPosition(gData.mDescription.mSelectText[i]), makePosition(dx, 0, 0)));
		addBlitzEntityPositionX(gData.mDescription.mSelect[i], dx);	
	}
}

static void setSelectActive() {
	changeBlitzMugenAnimation(gData.mDescription.mSelect[gData.mDescription.mCurrentSelection], 3001);

	gData.mDescription.mIsSelectActive = 1;
}

static void setSelectInactive() {
	changeBlitzMugenAnimation(gData.mDescription.mSelect[gData.mDescription.mCurrentSelection], 3000);
	setBlitzEntityScale2D(gData.mDescription.mSelect[gData.mDescription.mCurrentSelection], 1);

	gData.mDescription.mIsSelectActive = 0;
}

static void updateSelectActive() {
	if (!gData.mDescription.mStage) return;
	if (gData.mDescription.mIsSelectFinished) return;

	if (!gData.mDescription.mIsSelectActive && hasPressedAFlank()) {
		setSelectActive();
		playMugenSound(&gData.mSounds, 1, 0);
	}

	if (gData.mDescription.mIsSelectActive && hasPressedBFlank()) {
		playMugenSound(&gData.mSounds, 2, 0);
		setSelectInactive();
	}
}

static void updateActiveSelectionEffect() {
	if (!gData.mDescription.mIsSelectActive) return;
	if (gData.mDescription.mIsSelectFinished) return;

	int id = gData.mDescription.mSelect[gData.mDescription.mCurrentSelection];
	setBlitzEntityScale2D(id, getBlitzEntityScale(id).x + gData.mDescription.mSelectDX);

	double current = getBlitzEntityScale(id).x;
	if (current > 1.1 && gData.mDescription.mSelectDX > 0)  gData.mDescription.mSelectDX *= -1;
	if (current < 1.0 && gData.mDescription.mSelectDX < 0)  gData.mDescription.mSelectDX *= -1;
}

static void changeSelection(int tNew) {
	setSelectInactive();
	gData.mDescription.mCurrentSelection = tNew;
	setSelectActive();
}

static void updateSelection() {
	if (!gData.mDescription.mIsSelectActive) return;
	if (gData.mDescription.mIsSelectFinished) return;

	if (hasPressedUpFlank()) {
		int next = gData.mDescription.mCurrentSelection ? gData.mDescription.mCurrentSelection - 1 : 2;
		changeSelection(next);
		playMugenSound(&gData.mSounds, 3, 0);
	} 
	else if (hasPressedDownFlank()) {
		int next = (gData.mDescription.mCurrentSelection + 1) % 3;
		changeSelection(next);
		playMugenSound(&gData.mSounds, 3, 0);
	}
}

static void setSelectFinished() {
	setBlitzEntityScale2D(gData.mDescription.mSelect[gData.mDescription.mCurrentSelection], 1);
	gData.mDescription.mIsSelectFinished = 1;
}

static void setSelectUnfinished() {
	gData.mDescription.mIsSelectFinished = 0;
}

static void getGeneralPath(char* tDst, int i) {
	int which = randfromInteger(1, 2);
	sprintf(tDst, "story/SCENARIOS/GEN%d%d.def", i, which);
}

static void gotoDisaster(void* tCaller) {
	Country* country = &gData.mCountries[gData.mDescription.mCurrentDisplayedCountry];
	char path[1024];

	if (!country->mHasUsedCustom[0]) {
		sprintf(path, "story/SCENARIOS/%s1.def", country->mShort);
		country->mHasUsedCustom[0] = 1;
	}
	else {
		getGeneralPath(path, 1);
	}

	setCurrentStoryDefinitionFile(path, 5);
	setNewScreen(&StoryScreen);
}

static void gotoEducation(void* tCaller) {
	Country* country = &gData.mCountries[gData.mDescription.mCurrentDisplayedCountry];
	char path[1024];

	if (!country->mHasUsedCustom[1]) {
		sprintf(path, "story/SCENARIOS/%s2.def", country->mShort);
		country->mHasUsedCustom[1] = 1;
	}
	else {
		getGeneralPath(path, 2);
	}

	setCurrentStoryDefinitionFile(path, 5);
	setNewScreen(&StoryScreen);
}

static void gotoWar(void* tCaller) {
	Country* country1 = &gData.mCountries[gData.mDescription.mWarFirst];
	char path[1024];

	if (!country1->mHasUsedCustom[2]) {
		sprintf(path, "story/SCENARIOS/%s3.def", country1->mShort);
		country1->mHasUsedCustom[2] = 1;
	}
	else {
		getGeneralPath(path, 3);
	}

	setCurrentStoryDefinitionFile(path, 5);
	setNewScreen(&StoryScreen);
}



static void updateSelectFinished() {
	if (!gData.mDescription.mIsSelectActive) return;
	if (gData.mDescription.mIsFadingOut) return;

	if (!gData.mDescription.mIsSelectFinished && hasPressedAFlank()) {
		setSelectFinished();
		playMugenSound(&gData.mSounds, 1, 0);

		if (gData.mDescription.mCurrentSelection == 0) {
			addFadeOut(30, gotoDisaster, NULL);
			gData.mDescription.mIsFadingOut = 1;
		} else if (gData.mDescription.mCurrentSelection == 1) {
			addFadeOut(30, gotoEducation, NULL);
			gData.mDescription.mIsFadingOut = 1;
		}
		else {
			gData.mDescription.mWarFirst = gData.mDescription.mCurrentDisplayedCountry;
			gData.mDescription.mIsPickingWar = 1;
		}

	}

	if (gData.mDescription.mIsSelectFinished && hasPressedBFlank()) {
		setSelectUnfinished();
		playMugenSound(&gData.mSounds, 2, 0);
		gData.mDescription.mIsPickingWar = 0;
	}
}



static void updateWarSelectFinished() {
	if (!gData.mDescription.mIsPickingWar) return;
	if (gData.mDescription.mIsFadingOut) return;

	int isValidCountry = gData.mDescription.mRealCurrentCountry != -1 && gData.mDescription.mRealCurrentCountry != gData.mDescription.mWarFirst;
	if (hasPressedAFlank() && isValidCountry) {
		playMugenSound(&gData.mSounds, 1, 0);
		gData.mDescription.mWarSecond = gData.mDescription.mRealCurrentCountry;
		addFadeOut(30, gotoWar, NULL);
		gData.mDescription.mIsFadingOut = 1;
	}
}

static void updateWinLose() {
	if (gData.mHasWon) {
		gotoWin();
	}
	else if (gData.mHasLost) {
		gotoLose();
	}
}

static void updateDescriptionMenu() {
	updateDescriptionActive();
	updateDescriptionPosition();
	updateSelectActive();
	updateActiveSelectionEffect();
	updateSelection();
	updateSelectFinished();
	updateWarSelectFinished();
	updateWinLose();
}

static void updateWorldScreen() {
	updateWorldMovement();
	updateCrosshairRotation();
	updateDescriptionMenu();
}

Screen WorldScreen = {
	.mLoad = loadWorldScreen,
	.mUpdate = updateWorldScreen,
};

void resetDays() {
	gData.mDay = 0;
}

void addCurrentPopulation(int tVal) {
	gData.mCountries[gData.mDescription.mRealCurrentCountry].mPopulation += tVal;

}
void addCurrentGrowth(int tVal) {
	gData.mCountries[gData.mDescription.mRealCurrentCountry].mGrowth += tVal;

}
void addWarValues(int tPop1, int tPop2, int tGrowth1, int tGrowth2) {
	gData.mCountries[gData.mDescription.mWarFirst].mPopulation += tPop1;
	gData.mCountries[gData.mDescription.mWarSecond].mPopulation += tPop2;

	gData.mCountries[gData.mDescription.mWarFirst].mGrowth += tGrowth1;
	gData.mCountries[gData.mDescription.mWarSecond].mGrowth += tGrowth2;
}

char* getShortFirstWarCountry() {
	return gData.mCountries[gData.mDescription.mWarFirst].mShort;
}
char* getShortSecondWarCountry() {
	return gData.mCountries[gData.mDescription.mWarSecond].mShort;
}
