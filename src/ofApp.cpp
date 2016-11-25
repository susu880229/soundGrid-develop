#include "ofApp.h"

const int N = 256;		//Number of bands in spectrum
float spectrum[ N ];	//Smoothed spectrum values
float Rad = 500;		//Cloud raduis parameter
float Vel = 0.1;		//Cloud points velocity parameter
int bandRad = 2;		//Band index in spectrum, affecting Rad value
int bandVel = 100;		//Band index in spectrum, affecting Vel value

const int n = 300;		//Number of cloud points

//Offsets for Perlin noise calculation for points
float tx[n], ty[n];
ofPoint p[n];			//Cloud's points positions

float time0 = 0;		//Time value, used for dt computing

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_VERBOSE);

    // enable depth->video image calibration
    kinect.setRegistration(true);

    kinect.init();
    kinect.open();

    // print the intrinsic IR sensor values
    if(kinect.isConnected()) {
        ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
        ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
        ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
        ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
    }

    colorImg.allocate(kinect.width, kinect.height);
    grayImage.allocate(kinect.width, kinect.height);
    grayThreshNear.allocate(kinect.width, kinect.height);
    grayThreshFar.allocate(kinect.width, kinect.height);

    nearThreshold = 230.175;
    farThreshold = 130.4;
    bThreshWithOpenCV = true;

    ofSetFrameRate(60);

    // zero the tilt on startup
    angle = 3;
    kinect.setCameraTiltAngle(angle);

    kpt.loadCalibration("calibration_data/calibration.xml");

    //sounds
    for (int i = 0; i < NPLANETS; i++) {
        for (int j = 0; j < NTRACKS; j++) {
            sounds[i][j].load("sounds/" + to_string(i) + "_" + to_string(j) + ".mp3");
            sounds[i][j].setLoop(true);
        }
    }

    //osc
    sender.setup(IP_ADDRESS, PORT);
    receiver.setup(12000);

    // grid
    gridBG.load("sprites/gridBackground.png");
    front_grid.load("sprites/front_grid.png");
    front_glow.load("sprites/front_glow.png");
    headline.load("sprites/Headline.png");
    instruction.load("sprites/Instruction.png");
    code.load("sprites/Code.png");
    radioWave.load("sprites/Radiowave.png");
    frame.load("sprites/Frame.png");
    ring.load("sprites/Ring.png");
    star_back.load("sprites/Stars.png");
    for(int i=0; i<NGRIDS; i++){
        grids[i].setup(i);
    }

    // fonts
    font.load("fonts/BEBAS.ttf", 42);

    // state
    state = "start";
    start.load("sprites/start.png");


    // sound
    //Set spectrum values to 0
    for (int i=0; i<N; i++) {
        spectrum[i] = 0.0f;
    }

    //Initialize points offsets by random numbers
    for ( int j=0; j<n; j++ ) {
        tx[j] = ofRandom( 0, 1000 );
        ty[j] = ofRandom( 0, 1000 );
    }
    // not start showing animation on front screen
    animation = false;
    for (int i = 0; i < 9; i++) {
        video[i].load("videos/" + to_string(i) + ".mp4");
        planet_name[i].load("sprites/planets_name/" + to_string(i) + ".png");
    }
    //set currentposition = 4 to avoid show the first vedio at the beginning
    currentPosition = 4;
    video[currentPosition].play();
    probe.load("videos/Probe.mp4");
    probe.play();
    soundWave.load("videos/Soundwave.mp4");
    soundWave.play();
    //reset();
   // show = false;
    up = false;
    timer = 0;
    
    


}

//--------------------------------------------------------------
void ofApp::update()
{

    ofBackground(100, 100, 100);

    kinect.update();
    ofSoundUpdate();

    // there is a new frame and we are connected
    if(kinect.isFrameNew()) {

        // load grayscale depth image from the kinect source
        grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);

        grayThreshNear = grayImage;
        grayThreshFar = grayImage;
        grayThreshNear.threshold(nearThreshold, true);
        grayThreshFar.threshold(farThreshold);
        cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);

        // update the cv images
        grayImage.flagImageChanged();

        contourFinder.setMinArea(1000);
        contourFinder.setMaxArea(70000);
        contourFinder.setSortBySize(true);
        contourFinder.findContours(grayImage);

    }

    //osc receiver
    while (receiver.hasWaitingMessages()) {
        ofxOscMessage receivedMessage;
        receiver.getNextMessage(&receivedMessage);

        if (receivedMessage.getAddress() == "/digital/8") {
            int digital = receivedMessage.getArgAsInt(0);
            ofLogNotice() << digital;
        }
    }

    // grids
    for (int i = 0; i < NGRIDS; i++ ) {
        grids[i].update();
    }

    // sound
    //Get current spectrum with N bands
    float *val = ofSoundGetSpectrum( N );
    //We should not release memory of val,
    //because it is managed by sound engine

    //Update our smoothed spectrum,
    //by slowly decreasing its values and getting maximum with val
    //So we will have slowly falling peaks in spectrum
    for ( int i=0; i<N; i++ ) {
        spectrum[i] *= 0.97;	//Slow decreasing
        spectrum[i] = max( spectrum[i], val[i] );
    }

    //Update particles using spectrum values

    //Computing dt as a time between the last
    //and the current calling of update()
    float time = ofGetElapsedTimef();
    float dt = time - time0;
    dt = ofClamp( dt, 0.0, 0.1 );
    time0 = time; //Store the current time

    //Update Rad and Vel from spectrum
    //Note, the parameters in ofMap's were tuned for best result
    //just for current music track
    Rad = ofMap( spectrum[ bandRad ], 1, 3, 400, 800, true );
    Vel = ofMap( spectrum[ bandVel ], 0, 0.1, 0.05, 0.5 );

    //Update particles positions
    for (int j=0; j<n; j++) {
        tx[j] += Vel * dt;	//move offset
        ty[j] += Vel * dt;	//move offset
        //Calculate Perlin's noise in [-1, 1] and
        //multiply on Rad
        p[j].x = ofSignedNoise( tx[j] ) * Rad;
        p[j].y = ofSignedNoise( ty[j] ) * Rad;
    }
    //update video
    video[currentPosition].update();
    probe.update();
    soundWave.update();
    //move and scale the planet video
    /*
    if(pos_co < pos_cd)
    {
        pos_co += ofGetLastFrameTime() * moveSpeed;
        
        c_r += ofGetLastFrameTime() * upSpeed;
    }
    if(pos_co >=  pos_cd)
    {
        wave = true;
    }
    */
    //update timer for flash instruction
    if(state == "play" && currentPosition == 4)
    {
        if(currentPosition != originalPosition)
        {
            timer = 0;
            up = false;
        }
        blinkTimer();
        
    }
    if(currentPosition != 4)
    {
        up = false;
    }

}

void ofApp::sendMessage(string m) {
    ofxOscMessage message;
    message.setAddress("/songName");
    message.addStringArg(m);
    sender.sendMessage(message);
    ofLogNotice() << m;
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofSetColor(255, 255, 255);


    // draw from the live kinect
    kinect.drawDepth(kinect.width + 10, 0, kinect.width/2, kinect.height/2);
    kinect.draw(kinect.width + 10, kinect.height/2 + 10, kinect.width/2, kinect.height/2);

    //grayImage.draw(0, 0, kinect.width, kinect.height);

    //https://forum.openframeworks.cc/t/opencv-problem-finding-blob-centroid/16949/4
    grayImage.draw(0, 0);
    contourFinder.draw();


    // draw instructions
    ofSetColor(255, 255, 255);
    stringstream reportStream;

    if(kinect.hasAccelControl()) {
        reportStream << "accel is: " << ofToString(kinect.getMksAccel().x, 2) << " / "
        << ofToString(kinect.getMksAccel().y, 2) << " / "
        << ofToString(kinect.getMksAccel().z, 2) << endl;
    } else {
        reportStream << "Note: this is a newer Xbox Kinect or Kinect For Windows device," << endl
        << "motor / led / accel controls are not currently supported" << endl << endl;
    }

//    reportStream << "using opencv threshold = " << bThreshWithOpenCV <<" (press spacebar)" << endl
//    << "set near threshold " << nearThreshold << " (press: + -)" << endl
//    << "set far threshold " << farThreshold << " (press: < >) num blobs found " << contourFinder.nBlobs
//    << ", fps: " << ofGetFrameRate() << endl
//    << "press c to close the connection and o to open it again, connection is: " << kinect.isConnected() << endl;

    if(kinect.hasCamTiltControl()) {
        reportStream << "press UP and DOWN to change the tilt angle: " << angle << " degrees" << endl
        << "press 1-5 & 0 to change the led mode" << endl;
    }

    ofDrawBitmapString(reportStream.str(), 20, 652);
}

//--------------------------------------------------------------
void ofApp::drawGroundWindow (ofEventArgs & args)
{
    ofSetBackgroundColor(0, 0, 0);

    if (state == "start") {
        start.draw(364, 236, 236, 236);
    } else if (state == "play") {
        
        gridBG.draw(128, 0);
        /*
        for(int i=0; i<NGRIDS; i++){
            grids[i].draw();
        }
        */
        
        }
    

    RectTracker& tracker = contourFinder.getTracker();
    for(int i = 0; i < contourFinder.size(); i++) {
        vector<cv::Point> points = contourFinder.getContour(i);
        int label = contourFinder.getLabel(i);
        ofPoint center = toOf(contourFinder.getCenter(i));
        int age = tracker.getAge(label);

        //ofSetColor(ofColor::green);
        ofVec3f worldPoint = kinect.getWorldCoordinateAt(center.x, center.y);
        ofVec2f projectedPoint = kpt.getProjectedPoint(worldPoint);
        ofDrawCircle(GROUND_PROJECTOR_RESOLUTION_X * projectedPoint.x, GROUND_PROJECTOR_RESOLUTION_Y * projectedPoint.y, 50);
        //ofLog() << projectedPoint << endl;

        ofVec2f point = ofVec2f (projectedPoint.x * GROUND_PROJECTOR_RESOLUTION_X, projectedPoint.y * GROUND_PROJECTOR_RESOLUTION_Y);
        checkPoint(point);
        return;
    }


    //=======UNCOMMENT THIS PART TO TEST RESPONDING GRIDS========
    drawDot();

}

void ofApp::drawDot()
{
    int x = ofGetMouseX();
    int y = ofGetMouseY();

    // TODO:: why uncomment it will cover the screen
    //ofSetColor(0, 0, 230);
    ofDrawCircle(x, y, 5);
    checkPoint(ofVec2f (x, y));
}

void ofApp::drawFrontWindow(ofEventArgs& args)
{
    // visualize sound part
    /*
    for (int i = 0; i < NPLANETS; i++) {
        if (sounds[i][0].isPlaying()) {
            ofBackground( 255, 255, 255 );	//Set up the background

            //Draw background rect for spectrum
            ofSetColor( 230, 230, 230 );
            ofFill();
            ofDrawRectangle( 10, 700, N * 6, -100 );

            //Draw spectrum
            ofSetColor( 0, 0, 0 );
            for (int i=0; i<N; i++) {
                //Draw bandRad and bandVel by black color,
                //and other by gray color
                if ( i == bandRad || i == bandVel ) {
                    ofSetColor( 0, 0, 0 ); //Black color
                }
                else {
                    ofSetColor( 128, 128, 128 ); //Gray color
                }
                ofDrawRectangle( 10 + i * 5, spectrum[i] * 1000, 3, 3 );
            }

            //Draw cloud

            //Move center of coordinate system to the screen center
            ofPushMatrix();
            ofTranslate( ofGetWidth() / 2, ofGetHeight() / 2 );

            //Draw points
            ofSetColor( 0, 0, 0 );
            ofFill();
            for (int i=0; i<n; i++) {
                ofDrawCircle( p[i], 2 );
            }

            //Draw lines between near points
            float dist = 40;	//Threshold parameter of distance
            for (int j=0; j<n; j++) {
                for (int k=j+1; k<n; k++) {
                    if ( ofDist( p[j].x, p[j].y, p[k].x, p[k].y )
                        < dist ) {
                        ofDrawLine( p[j], p[k] );
                    }
                }
            }

            //Restore coordinate system
            ofPopMatrix();
        }
    }
    */



    //add animation part according to the currentposition
    ofBackground( 0, 0, 0, 128 );
    star_back.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
    
    //before play state
    frame.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
    probe.draw(350, 314, 70, 140);
    front_grid.draw(700, 284, 200, 200);
    headline.draw(290, 95, 430, 40);
    ring.draw(100, 284, 200, 200);
    //give player instruction to step on grid
    if(up == true)
    {
        instruction.draw(300, 125, 400, 50 );
        
    }
    
    //after step on any grid
    if(animation == true)
    {
        //avoid the repeat playing static video without animation
        if(!video[currentPosition].isPlaying())
        {
            
            video[currentPosition].play();
            
        }
        //put draw outside to avoid disppear
        video[currentPosition].draw(100, 284, 200, 200);
        planet_name[currentPosition].draw(150, 550, 100, 25);
        front_glow.draw(700 + ((currentPosition % 3) * 200 / 3), 284 + (floor(currentPosition / 3) * 200 / 3) , 200 / 3, 200/ 3 );
        if(currentPosition != 4)
        {
            radioWave.draw(250, 284, 150, 200);
            soundWave.draw(420, 334, 280, 100);
            code.draw(350, 570, 420, 100);
        }

    }
    
    if(currentPosition == 4)
    {
        front_glow.draw(700 + 200 / 3, 284 + 200 / 3, 200 / 3, 200 / 3);
        
    }
    
    
    
    
    
    
    
    //draw has to be outside of condition otherwise only static pictures

    //draw vedio
    /*
    ofBackground( 0, 0 , 0, 128);
    video[currentPosition].draw((currentPosition % 3) * (ofGetWindowWidth() / 3) + 50, floor(currentPosition / 3) * (ofGetWindowHeight() / 3) + 20);
    //draw lines

    ofSetColor(255,255,255);
    ofFill();
    ofDrawLine(0, ofGetWindowHeight() / 3, ofGetWindowWidth(), ofGetWindowHeight() / 3);
    ofDrawLine(0, (ofGetWindowHeight() / 3) * 2, ofGetWindowWidth(), (ofGetWindowHeight() / 3) * 2);
    ofDrawLine(ofGetWindowWidth() / 3, 0, ofGetWindowWidth() / 3, ofGetWindowHeight());
    ofDrawLine((ofGetWindowWidth() / 3) * 2, 0, (ofGetWindowWidth() / 3) * 2, ofGetWindowHeight());
     */
}

//not put any local varible to time, will cause problem
void ofApp::blinkTimer()
{
    
    //timer
    if(timer <= 0)
    {
        up = true;
    }
    if (timer >= 1)
    {
        up = false;
    }
    if(up == true)
    {
        timer += ofGetLastFrameTime() * 10;
    }
    if (up == false)
    {
        timer -= ofGetLastFrameTime() * 10;
    }
    //ofLogNotice() << timer << endl;
}

//--------------------------------------------------------------
void ofApp::exit()
{
    kinect.setCameraTiltAngle(3); // zero the tilt on exit
    kinect.close();
}

//--------------------------------------------------------------
void ofApp::exitGroundWindow(ofEventArgs &args)
{

}

//--------------------------------------------------------------
void ofApp::keyPressed (int key)
{
    switch (key) {
        case ' ':
            bThreshWithOpenCV = !bThreshWithOpenCV;
            break;

        case '>':
        case '.':
            farThreshold ++;
            if (farThreshold > 255) farThreshold = 255;
            break;

        case '<':
        case ',':
            farThreshold --;
            if (farThreshold < 0) farThreshold = 0;
            break;

        case '+':
        case '=':
            nearThreshold ++;
            if (nearThreshold > 255) nearThreshold = 255;
            break;

        case '-':
            nearThreshold --;
            if (nearThreshold < 0) nearThreshold = 0;
            break;

        case 'w':
            kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
            break;

        case 'o':
            kinect.setCameraTiltAngle(angle); // go back to prev tilt
            kinect.open();
            break;

        case 'c':
            kinect.setCameraTiltAngle(0); // zero the tilt
            kinect.close();
            break;

        case '1':
            kinect.setLed(ofxKinect::LED_GREEN);
            break;

        case '2':
            kinect.setLed(ofxKinect::LED_YELLOW);
            break;

        case '3':
            kinect.setLed(ofxKinect::LED_RED);
            break;

        case '4':
//            kinect.setLed(ofxKinect::LED_BLINK_GREEN);
            sendMessage("hahahahah");
            break;

        case '5':
//            kinect.setLed(ofxKinect::LED_BLINK_YELLOW_RED);
            ofLogNotice() << "QQQQQQ" << kinectPoint;
            break;

        case '0':
            kinect.setLed(ofxKinect::LED_OFF);
            break;

        case OF_KEY_UP:
            angle++;
            if(angle>30) angle=30;
            kinect.setCameraTiltAngle(angle);
            break;

        case OF_KEY_DOWN:
            angle--;
            if(angle<-30) angle=-30;
            kinect.setCameraTiltAngle(angle);
            break;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{

}

void ofApp::mouseMovedGroundWindow (ofMouseEventArgs& args)
{

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void ofApp::checkPoint(ofVec2f point)
{
    int cp;
    float dt;



    if (state == "start") {

        if (point.x >= (128 + 236 + 20) && point.x <= (128 + 236 + 20 + 216) && point.y >= (236 + 20) && point.y <= (236 + 20 + 216)) {
            cp = 1;
        } else {
            cp = 0;
        }

        if (originalPosition != cp) {
            originalPosition = cp;

            if (cp == 1) {
                startTime = ofGetElapsedTimef();
            }

        } else if (cp == 1 && originalPosition == cp) {
            dt = ofGetElapsedTimef() - startTime;
            if (dt >= TIME_DELAY) {
                
                state = "play";
            }
        }


    } else if (state == "play") {
        for (int i = 0; i < NGRIDS; i++) {
            cp = grids[i].getCurrentPosition(point);
            if (cp >= 0) {
                currentPosition = cp;
            }
        }

        // Leave grids reset timer
        if (cp == -2) {
            for (int i = 0; i < NGRIDS; i++) {
                grids[i].reset();
            }
            stopSound();
            animation = false;
            //reset();

            // New timer when jump into another different grid
        } else if (originalPosition != currentPosition) {
            stopSound();
            startTime = ofGetElapsedTimef();
            originalPosition = currentPosition;
            animation = false;
            //reset();

            // Still there
        } else if (originalPosition == currentPosition) {
            dt = ofGetElapsedTimef() - startTime;

            if(dt >= TIME_DELAY) {
                playSound();
                grids[currentPosition].light();
                animation = true;
            }

        }
    }
}

void ofApp::playSound()
{
    if (!sounds[currentPosition][0].isPlaying()) {
        sounds[currentPosition][0].play();
    }
}

void ofApp::stopSound()
{
    for (int i = 0; i < NPLANETS; i ++) {
        for (int j = 0; j < NTRACKS; j++) {
            if (sounds[i][j].isPlaying()) {
                sounds[i][j].stop();
            }
        }
    }
}
/*
void ofApp::reset()
{
    pos_co = 0;
    pos_cd = 284;
    c_r = 100;
    wave = false;
}
 */

