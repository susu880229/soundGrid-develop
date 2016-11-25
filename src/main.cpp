#include "ofMain.h"
#include "ofApp.h"
#include "ofAppGLFWWindow.h"

//========================================================================
int main( ){
    
    ofGLFWWindowSettings settings;
    //set up mainWindow (Kinect)
    settings.width = 1024;
    settings.height = 768;
    settings.setPosition(ofVec2f(100, 100));
    settings.resizable = true;
    shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);
    
    //set up groundWindow
    settings.width = GROUND_PROJECTOR_RESOLUTION_X;
    settings.height = GROUND_PROJECTOR_RESOLUTION_Y;
    //settings.setPosition(ofVec2f(ofGetScreenWidth(), 0));
    settings.setPosition(ofVec2f(200, 0));
    //settings.resizable = false;
    //settings.decorated = false;
    settings.shareContextWith = mainWindow;
    shared_ptr<ofAppBaseWindow> groundWindow = ofCreateWindow(settings);
    groundWindow->setVerticalSync(false);
    
    //set up frontWindow
    settings.width = FRONT_PROJECTOR_RESOLUTION_X;
    settings.height = FRONT_PROJECTOR_RESOLUTION_Y;
    settings.setPosition(ofVec2f(400, 0));
    settings.shareContextWith = mainWindow;
    shared_ptr<ofAppBaseWindow> frontWindow = ofCreateWindow(settings);
    frontWindow->setVerticalSync(false);
    
    //replace the draw method with drawFrontWindow method instead
    shared_ptr<ofApp> mainApp(new ofApp);
    ofAddListener(groundWindow->events().draw, mainApp.get(), &ofApp::drawGroundWindow);
    ofAddListener(groundWindow->events().exit, mainApp.get(), &ofApp::exitGroundWindow);
    ofAddListener(groundWindow->events().mouseMoved, mainApp.get(),
                  &ofApp::mouseMovedGroundWindow);
    ofAddListener(frontWindow->events().draw, mainApp.get(), &ofApp::drawFrontWindow);
    
    ofRunApp(mainWindow, mainApp);
    ofRunMainLoop();

}
