//
//  Grid.h
//  soundGrid
//
//  Created by Xiao Chen on 2016-11-02.
//
//

#ifndef _Grid
#define _Grid

#include "ofMain.h"

#define GROUND_PROJECTOR_RESOLUTION_X 1024
#define GROUND_PROJECTOR_RESOLUTION_Y 768

class Grid {
    
public:
    
    void setup(int _pos);	// setup method, use this to setup your object's initial state
    void generateGrids();
    void update();  // update method, used to refresh your objects properties
    void draw();    // draw method, this where you'll do the object's drawing
    int getCurrentPosition(ofVec2f point);
    void light();
    void reset();
    
    // variables
    float x;        // position
    float y;
    float speedY;   // speed and direction
    float speedX;
    int dim;        // size
    ofColor color;  // color using ofColor type
    
    Grid();
    
//private:
    
    float margin = 20;
    float side = 216;
    int gridPos;
    
    ofRectangle outerRect;
    ofPath rectPath;
    ofPath internalPath;
    ofVec2f originalPos;
    
    float mainMargin;
    string mode;
    
    ofImage gridGlow;
    ofVideoPlayer video;
};

#endif /* Grid_h */
