#include "storyscreen.h"

#include <assert.h>

#include <prism/blitz.h>


#include "titlescreen.h"
#include "worldscreen.h"

static struct {
	MugenDefScript mScript;
	MugenDefScriptGroup* mCurrentGroup;
	MugenSpriteFile mSprites;
	MugenSounds mSounds;

	MugenAnimation* mOldAnimation;
	MugenAnimation* mAnimation;
	int mAnimationID;
	int mOldAnimationID;

	Position mOldAnimationBasePosition;
	Position mAnimationBasePosition;

	int mSpeakerID;
	int mTextID;

	int mIsStoryOver;

	char mDefinitionPath[1024];
	int mTrack;
} gData;

static int isImageGroup() {
	char* name = gData.mCurrentGroup->mName;
	char firstW[100];
	sscanf(name, "%s", firstW);

	return !strcmp("Image", firstW);
}

static void increaseGroup() {
	gData.mCurrentGroup = gData.mCurrentGroup->mNext;
}

static void loadImageGroup() {
	if (gData.mOldAnimationID != -1) {
		removeMugenAnimation(gData.mOldAnimationID);
		destroyMugenAnimation(gData.mOldAnimation);
	}

	if (gData.mAnimationID != -1) {
		setMugenAnimationBasePosition(gData.mAnimationID, &gData.mOldAnimationBasePosition);
	}

	gData.mOldAnimationID = gData.mAnimationID;
	gData.mOldAnimation = gData.mAnimation;


	int group = getMugenDefNumberVariableAsGroup(gData.mCurrentGroup, "group");
	int item = getMugenDefNumberVariableAsGroup(gData.mCurrentGroup, "item");
	gData.mAnimation = createOneFrameMugenAnimationForSprite(group, item);
	gData.mAnimationID = addMugenAnimation(gData.mAnimation, &gData.mSprites, makePosition(0, 0, 0));
	setMugenAnimationBasePosition(gData.mAnimationID, &gData.mAnimationBasePosition);

	increaseGroup();
}



static int isPopulationGroup() {
	char* name = gData.mCurrentGroup->mName;
	char firstW[100];
	sscanf(name, "%s", firstW);

	return !strcmp("Population", firstW);
}

static void loadPopulationGroup() {
	
	int value = getMugenDefIntegerOrDefaultAsGroup(gData.mCurrentGroup, "value", 0);
	addCurrentPopulation(value);

	increaseGroup();
}

static int isGrowthGroup() {
	char* name = gData.mCurrentGroup->mName;
	char firstW[100];
	sscanf(name, "%s", firstW);

	return !strcmp("Growth", firstW);
}

static void loadGrowthGroup() {

	int value = getMugenDefIntegerOrDefaultAsGroup(gData.mCurrentGroup, "value", 0);
	addCurrentGrowth(value);

	increaseGroup();
}

static int isWarGroup() {
	char* name = gData.mCurrentGroup->mName;
	char firstW[100];
	sscanf(name, "%s", firstW);

	return !strcmp("War", firstW);
}

static void loadWarGroup() {

	int pop1 = getMugenDefIntegerOrDefaultAsGroup(gData.mCurrentGroup, "pop1", 0);
	int pop2 = getMugenDefIntegerOrDefaultAsGroup(gData.mCurrentGroup, "pop2", 0);
	int growth1 = getMugenDefIntegerOrDefaultAsGroup(gData.mCurrentGroup, "growth1", 0);
	int growth2 = getMugenDefIntegerOrDefaultAsGroup(gData.mCurrentGroup, "growth2", 0);

	addWarValues(pop1, pop2, growth1, growth2);

	increaseGroup();
}

static int isTextGroup() {
	char* name = gData.mCurrentGroup->mName;
	char firstW[100];
	sscanf(name, "%s", firstW);

	return !strcmp("Text", firstW);
}

static void parseText(char* tDst, char* tSrc) {

	char* s = tSrc;
	char* d = tDst;

	while (*s) {
		if (*s == '%' && (*(s + 1)) == '1') {
			sprintf(d, "%s", getShortFirstWarCountry());
			int n = strlen(d);
			s++;
			d += n;
		}
		else if (*s == '%' && (*(s + 1)) == '2') {
			sprintf(d, "%s", getShortSecondWarCountry());
			int n = strlen(d);
			s+=2;
			d += n;
		}
		else {
			*d = *s;
			d++;
			s++;
		}
	
		
	}
	*d = '\0';

	printf("%s\n", tDst);
}

static void loadTextGroup() {
	if (gData.mTextID != -1) {
		removeMugenText(gData.mTextID);
		removeMugenText(gData.mSpeakerID);
	}

	char* speaker = getAllocatedMugenDefStringVariableAsGroup(gData.mCurrentGroup, "speaker");
	char* text = getAllocatedMugenDefStringVariableAsGroup(gData.mCurrentGroup, "text");

	gData.mSpeakerID = addMugenText(speaker, makePosition(40, 348, 3), 1);


	char finalText[1000];
	parseText(finalText, text);

	gData.mTextID = addMugenText(finalText, makePosition(30, 380, 3), 1);
	setMugenTextBuildup(gData.mTextID, 1);
	setMugenTextTextBoxWidth(gData.mTextID, 560);
	setMugenTextColor(gData.mTextID, COLOR_WHITE);

	freeMemory(speaker);
	freeMemory(text);

	increaseGroup();
}

static int isTitleGroup() {
	char* name = gData.mCurrentGroup->mName;
	char firstW[100];
	sscanf(name, "%s", firstW);

	return !strcmp("Title", firstW);
}

static void goToTitle(void* tCaller) {
	(void)tCaller;
	setNewScreen(&TitleScreen);
}

static void loadTitleGroup() {
	gData.mIsStoryOver = 1;

	addFadeOut(30, goToTitle, NULL);
}

static int isWorldGroup() {
	char* name = gData.mCurrentGroup->mName;
	char firstW[100];
	sscanf(name, "%s", firstW);

	return !strcmp("World", firstW);
}

static void goToWorld(void* tCaller) {
	(void)tCaller;
	setNewScreen(&WorldScreen);
}

static void loadWorldGroup() {
	gData.mIsStoryOver = 1;
	addFadeOut(30, goToWorld, NULL);
}

static void loadNextStoryGroup() {
	int isRunning = 1;
	while (isRunning) {
		if (isImageGroup()) {
			loadImageGroup();
		}
		else if (isPopulationGroup()) {
			loadPopulationGroup();
		}
		else if (isGrowthGroup()) {
			loadGrowthGroup();
		}
		else if (isWarGroup()) {
			loadWarGroup();
		}
		else if (isTextGroup()) {
			loadTextGroup();
			break;
		}
		else if (isTitleGroup()) {
			loadTitleGroup();
			break;
		}
		else if (isWorldGroup()) {
			loadWorldGroup();
			break;
		}
		else {
			logError("Unidentified group type.");
			logErrorString(gData.mCurrentGroup->mName);
			abortSystem();
		}
	}
}

static void findStartOfStoryBoard() {
	gData.mCurrentGroup = gData.mScript.mFirstGroup;

	while (gData.mCurrentGroup && strcmp("STORYSTART", gData.mCurrentGroup->mName)) {
		gData.mCurrentGroup = gData.mCurrentGroup->mNext;
	}

	assert(gData.mCurrentGroup);
	gData.mCurrentGroup = gData.mCurrentGroup->mNext;
	assert(gData.mCurrentGroup);

	gData.mAnimationID = -1;
	gData.mOldAnimationID = -1;
	gData.mTextID = -1;

	gData.mOldAnimationBasePosition = makePosition(0, 0, 1);
	gData.mAnimationBasePosition = makePosition(0, 0, 2);

	loadNextStoryGroup();
}



static void loadStoryScreen() {
	gData.mIsStoryOver = 0;

	gData.mSounds = loadMugenSoundFile("world/WORLD.snd");

	gData.mScript = loadMugenDefScript(gData.mDefinitionPath);

	if (isMugenDefStringVariable(&gData.mScript, "Header", "sprites")) {
		char* spritePath = getAllocatedMugenDefStringVariable(&gData.mScript, "Header", "sprites");
		gData.mSprites = loadMugenSpriteFileWithoutPalette(spritePath);
		freeMemory(spritePath);
	}
	else {
		char path[1024];
		char folder[1024];
		strcpy(folder, gData.mDefinitionPath);
		char* dot = strrchr(folder, '.');
		*dot = '\0';
		sprintf(path, "%s.sff", folder);
		gData.mSprites = loadMugenSpriteFileWithoutPalette(path);

	}

	findStartOfStoryBoard();

	playTrack(gData.mTrack);
}


static void updateText() {
	if (gData.mIsStoryOver) return;
	if (gData.mTextID == -1) return;

	if (hasPressedAFlankSingle(0) || hasPressedAFlankSingle(1)) {
		if (isMugenTextBuiltUp(gData.mTextID)) {
			loadNextStoryGroup();
		}
		else {
			setMugenTextBuiltUp(gData.mTextID);
		}
	}
}

static void updateStoryScreen() {

	updateText();
}


Screen StoryScreen = {
	.mLoad = loadStoryScreen,
	.mUpdate = updateStoryScreen,
};


void setCurrentStoryDefinitionFile(char* tPath, int tTrack) {
	strcpy(gData.mDefinitionPath, tPath);
	gData.mTrack = tTrack;
}
