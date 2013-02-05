#include "testApp.h"
#include "ofAppGlutWindow.h"
#include "myUtils.h"

//--------------------------------------------------------------
int main(){
	ofAppGlutWindow window; // create a window
	// set width, height, mode (OF_WINDOW or OF_FULLSCREEN)
	//ofSetFullscreen(true);
	ofSetupOpenGL(&window, 1024, 768, OF_WINDOW);
	//ofSetFullscreen(true);

	ofRunApp(new testApp()); // start the app
}
