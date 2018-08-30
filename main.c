#include <prism/framerateselectscreen.h>
#include <prism/pvr.h>
#include <prism/physics.h>
#include <prism/file.h>
#include <prism/drawing.h>
#include <prism/log.h>
#include <prism/wrapper.h>
#include <prism/system.h>
#include <prism/stagehandler.h>
#include <prism/logoscreen.h>
#include <prism/mugentexthandler.h>

#include "worldscreen.h"
#include "storyscreen.h"
#include "titlescreen.h"

#ifdef DREAMCAST
KOS_INIT_FLAGS(INIT_DEFAULT);

extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

#endif


void exitGame() {
	shutdownPrismWrapper();

#ifdef DEVELOP
	abortSystem();
#else
	returnToMenu();
#endif
}

void setMainFileSystem() {
#ifdef DEVELOP
	setFileSystem("/pc");
#else
	setFileSystem("/cd");
#endif
}

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	setGameName("HELP");
	setScreenSize(640, 480);
	
	setMainFileSystem();	
	initPrismWrapperWithConfigFile("data/config.cfg");

	setFont("$/rd/fonts/segoe.hdr", "$/rd/fonts/segoe.pkg");

	logg("Check framerate");
	FramerateSelectReturnType framerateReturnType = selectFramerate();
	if (framerateReturnType == FRAMERATE_SCREEN_RETURN_ABORT) {
		exitGame();
	}

	setMemoryHandlerCompressionActive();
	addMugenFont(1, "3.fnt");
	loadCountries();
	setWrapperTitleScreen(&TitleScreen);
	setScreenAfterWrapperLogoScreen(getLogoScreenFromWrapper());
	setCurrentStoryDefinitionFile("story/INTRO/INTRO.def", 3);
	startScreenHandling(&StoryScreen);

	exitGame();
	
	return 0;
}


