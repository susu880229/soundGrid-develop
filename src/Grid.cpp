//
//  Grid.cpp
//  soundGrid
//
//  Created by Xiao Chen on 2016-11-02.
//
//

#include "Grid.h"

Grid::Grid()
{
}

void Grid::setup(int _pos)
{
    gridPos = _pos;
    mainMargin = (GROUND_PROJECTOR_RESOLUTION_X - GROUND_PROJECTOR_RESOLUTION_Y) / 2;
    generateGrids();
    
    gridGlow.load("sprites/gridGlow.png");
    if (gridPos != 4) {
        video.load("videos/" + to_string(gridPos) + ".mp4");
        video.play();
    }
}

void Grid::generateGrids()
{
    int outerSide = 2 * margin + side;
    
    // Set the original position for calculations
    originalPos.x = mainMargin + outerSide * (gridPos % 3) + margin;
    originalPos.y = outerSide * (floor(gridPos / 3)) + margin;

    //----------CALCULATE THE PATH FOR OUTER RECTANGLE------------//
    rectPath.rectangle(mainMargin + outerSide * (gridPos % 3), outerSide * (floor(gridPos / 3)), outerSide, outerSide);
    rectPath.setFillColor(ofColor::blue);
    rectPath.setFilled(false);
    rectPath.setStrokeWidth(0);
    
    //----------CALCULATE THE PATH FOR INTERNAL RECTANGLE------------//
    internalPath.rectangle(originalPos.x, originalPos.y, side, side);
    internalPath.setFillColor(ofColor::black);
    internalPath.setFilled(false);
    internalPath.setStrokeWidth(0);
    
}

void Grid::update()
{
    video.update();
}

void Grid::draw()
{
    if (mode == "init") {
        if (gridPos == 4) {
//            ofSetColor(255, 0, 0);
            ofDrawRectangle(mainMargin + (2 * margin + side), 2 * margin + side, 2 * margin + side, 2 * margin + side);
        }
    } else {
        //----------DRAW THE OUTER RECTANGLE BY PATH------------//
        rectPath.draw();
        
        //----------DRAW THE INTERNAL RECTANGLE------------//
        internalPath.draw();
    }
}

int Grid::getCurrentPosition(ofVec2f point)
{
    if (point.x >= originalPos.x
        && point.x <= originalPos.x + side
        && point.y >= originalPos.y
        && point.y <= originalPos.y + side) {
        return gridPos;
    } else if (point.x < mainMargin || point.x > mainMargin + GROUND_PROJECTOR_RESOLUTION_Y) {
        return -2;
    } else {
        reset();
        return -1;
    }
}

void Grid::light()
{
    //TODO: stupid function
    
    if(gridPos == 0) {
        gridGlow.draw(128, 0);
        video.draw(128 + 25, 25, 200, 200);
    } else if (gridPos == 1) {
        video.draw(128 + 250 + 9 + 25, 25, 200, 200);
        gridGlow.draw(128 + 250 + 9, 0);
    } else if (gridPos == 2) {
        video.draw(128 + 250 + 9 + 250 + 9 + 25, 25, 200, 200);
        gridGlow.draw(128 + 250 + 9 + 250 + 9, 0);
    } else if (gridPos == 3) {
        video.draw(128 + 25, 250 + 9 + 25, 200, 200);
        gridGlow.draw(128, 250 + 9);
    } else if (gridPos == 4) {
        gridGlow.draw(128 + 250 + 9, 250 + 9);
    } else if (gridPos == 5) {
        video.draw(128 + 250 + 9 + 250 + 9 + 25, 250 + 9 + 25, 200, 200);
        gridGlow.draw(128 + 250 + 9 + 250 + 9, 250 + 9);
    } else if (gridPos == 6) {
        gridGlow.draw(128, 250 + 9 + 250 + 9);
        video.draw(128 + 25, 250 + 9 + 250 + 9 + 25, 200, 200);
    } else if (gridPos == 7) {
        gridGlow.draw(128 + 250 + 9, 250 + 9 + 250 + 9);
        video.draw(128 + 250 + 9 + 25, 250 + 9 + 250 + 9 + 25, 200, 200);
    } else if (gridPos == 8) {
        gridGlow.draw(128 + 250 + 9 + 250 + 9, 250 + 9 + 250 + 9);
        video.draw(128 + 250 + 9 + 250 + 9 + 25, 250 + 9 + 250 + 9 + 25, 200, 200);
    }
     //rectPath.setFillColor(ofColor::green);
}

void Grid::reset()
{
    //rectPath.setFillColor(ofColor::blue);
}