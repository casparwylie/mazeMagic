//
//  main.cpp
//  mazeMagic
//
//  Created by Caspar Wylie on 22/08/2016.
//  Copyright © 2016 Caspar Wylie. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <ctime>
#include <unistd.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"


using namespace std;
using namespace cv;

Mat rotateMatrix(Mat frame, int amount){
    
    Mat rotatedMat;

    Point2f rotate2f = Point2f(frame.cols/2, frame.rows/2);
    rotatedMat = getRotationMatrix2D(rotate2f, amount, 1);
    warpAffine(frame, frame, rotatedMat, frame.size());
    
    
    return frame;
}

vector<int> findStartCoord(Mat frame){
    
    int sliceHeight = 1;
    Mat slice;
    int coverCount = 0;
    int yDown = 0;
    float minAreaForDetect = int((sliceHeight*frame.cols)*0.7);
    int searchPoint;
    bool startFound = false;
    vector<int> startData(5);
    Point startPoint;
    int forceRotateCount = 0;
    int count = 0;
    int diffWin = 0;
    
    while(startFound==false && count < 4){
        while(coverCount < minAreaForDetect){
            yDown = yDown + sliceHeight;
            diffWin++;
            coverCount = 0;
            if(yDown>=frame.rows){
                break;
                //error
            }
            slice = frame(Rect(0,yDown,frame.cols, sliceHeight));
            if(count==1){
                count = 1;
            }
            for(int rows = 0;rows < sliceHeight;rows++){
                for(int cols = 0; cols < frame.cols;cols++){
                    int currentPoint = slice.at<uchar>(rows,cols);
                    if(currentPoint<100){
                        coverCount++;
                    }
                }
            }
        }
        
        string searchState = "wait";
        
        
        int startX = 0;
        int endX = 0;
        if(coverCount>0){
            for(int cols = 0; cols < frame.cols;cols++){
                searchPoint = slice.at<uchar>(0,cols);
                if(searchPoint < 100 && searchState == "wait"){
                    searchState = "search";
                    
                }
                
                if(searchState=="search" && searchPoint > 100){
                    
                    if(startX==0){
                        startX = cols;
                        searchState = "found";
                    }
                    
                }else if(searchState=="found" && searchPoint < 100){
                    if(endX==0){
                        endX = cols;
                    }
                    break;
                }
            }
        }
        
        if(endX>0 && startX>0 && coverCount>0){
            
            int pathWidth = endX - startX;
            if(startX!=slice.cols){
                startPoint = Point(endX, yDown);
                startData[0] = startX;
                startData[1] = yDown;
                startData[2] = forceRotateCount;
                startData[3] = pathWidth;
                startData[4] = endX;
                startFound = true;
            }
        }else{
            
            frame = rotateMatrix(frame, 90);
            coverCount = 0;
            
            minAreaForDetect = int((sliceHeight*frame.cols)*0.7);
            forceRotateCount+=90;
            yDown = 0;
        }
        
        count++;
    }
    
    return startData;
}


Mat vect(Mat singleChannel, vector<int> startCoord, string side, bool over, Mat orgSingFrame){
    
    int drawToCurrDist = 3;
    
    string direction = "D";
    int currX;
    int currY = startCoord[1] + 1;
    int drawX;
    int drawY = startCoord[1] + 1;
    
    int pathWidth = startCoord[3];
    
    int currPoint;
    int currPointDraw;
    int drawSize = pathWidth/2;
    Mat drawFrame;
    
    Scalar drawColor;
    
    vector<string> crossDirs(4);
    
    if(side=="left"){
        currX = startCoord[0] - 1;
        drawX = startCoord[0] + drawToCurrDist;

        crossDirs = {"L", "R", "U", "D"};
        
        drawColor = Scalar(0);
        
    }else{
        
        currX = startCoord[4] + 1;
        drawX = startCoord[4] - drawToCurrDist;

        crossDirs = {"R", "L", "D", "U"};
        
        drawColor = Scalar(255);
    }
    

    
    int limitDim;
    
    drawFrame = singleChannel.clone();
    if(startCoord[2]>0){
        orgSingFrame = rotateMatrix(orgSingFrame, -1 * startCoord[2]);
        
        limitDim = currY;
        
    }else{
        limitDim = currX;
    }
    

    if(over==true){
        
        Mat findSCurr = drawFrame(Rect(startCoord[0],startCoord[1],40, 1));
        int searchPoint;
        int foundCurrX;
        for(int cols = 0; cols < findSCurr.cols;cols++){
            searchPoint = findSCurr.at<uchar>(0,cols);
            if(searchPoint>100){
                foundCurrX = (cols-1);
                break;
            }
        }

        drawColor = Scalar(50);
        drawSize = 2;
        currX  = startCoord[0] + foundCurrX;
        drawX = currX + drawToCurrDist;
      
        
    }
    

    for(int i = 0;i<300000;i++){
        
      
        if(over==true){
           
            
           // cout << direction;
            circle(orgSingFrame, Point(currX,currY), 2, Scalar(200));
            
            
            circle(drawFrame, Point(currX,currY), 1, Scalar(200));
            circle(drawFrame, Point(drawX,drawY), 1, Scalar(100));
        
            
        }else{
            rectangle(drawFrame, Point(drawX-drawSize,drawY-drawSize),Point(drawX+drawSize,drawY+drawSize), drawColor, -1);

        }
        
        currPoint = singleChannel.at<uchar>(currY, currX);
        currPointDraw = singleChannel.at<uchar>(drawY, drawX);
        
        
        if(direction == "D"){
            if(currPoint < 100){ //on path, go down.
                
                currY++;
                drawY++;
                if(currPointDraw < 100){ //hit corner
                    currX = drawX;
                    currY = drawY;
                    drawY -= drawToCurrDist;
                    
                    direction = crossDirs[0];
                    
                }
            }else{ // reached end of path.
                direction = crossDirs[1];
                currY--;
                drawY--;
                
                drawX = currX;
                drawY +=drawToCurrDist;
                
            }
        }else if(direction=="L"){
            if(currPoint < 100){ //on path, go left.
                
                currX++;
                drawX++;
                
                if(currPointDraw < 100){ //hit corner
                    
                    currX = drawX;
                    currY = drawY;
                    drawX -= drawToCurrDist;
                    
                    direction = crossDirs[2];
                }
            }else{ //reached end of path
                
                direction = crossDirs[3];
                currX--;
                drawY--;
                
                drawX += drawToCurrDist;
                drawY = currY;
                
            }
            
        }else if(direction=="U"){
            if(currPoint < 100){ //on path, go up.
                
                currY--;
                drawY--;
                if(currPointDraw < 100){ //hit corner
                    direction = crossDirs[1];
                    
                    
                    currX = drawX;
                    currY = drawY;
                    drawY += drawToCurrDist;
                }
                
            }else{
                
                direction = crossDirs[0];
                currY++;
                drawY--;
                
                drawX = currX;
                drawY -=drawToCurrDist;
                
            }
            
        }else if(direction=="R"){
            if(currPoint < 100){ //on path, go right.
                
                currX--;
                drawX--;
                if(currPointDraw < 100){ //hit corner
                    direction = crossDirs[3];
                    currY = drawY;
                    currX = drawX;
                    
                    drawX += drawToCurrDist;
                }
                
            }else{
                direction = crossDirs[2];
                
                currX++;
                drawY = currY;
                drawX -= drawToCurrDist;
                
                
            }
        }
    }
    
    if(over==false){
        return drawFrame;
    }else{
        return orgSingFrame;
    }
}

Mat solve(Mat orgFrame){
    
    Mat singleChannel;
    
    cvtColor(orgFrame, singleChannel, CV_BGR2GRAY);
    threshold(singleChannel, singleChannel, 127, 255, THRESH_BINARY);
    
    Mat orgSingFrame = orgFrame;
    
    vector<int> startCoord = findStartCoord(singleChannel);
    
    Mat backLineL = vect(singleChannel,startCoord, "right", false, orgSingFrame);
    
    Mat backLineBoth = vect(backLineL,startCoord, "left", false, orgSingFrame);
    
    imshow("3", backLineBoth);
    
    Mat final = vect(backLineBoth, startCoord, "left", true, orgSingFrame);
    
    if(startCoord[2]>0){
        final = rotateMatrix(final, -1 * startCoord[2]);
    }
    return final;
}

int main(int argc, const char * argv[]) {
    
    
    
   // for(int picID = 1;picID<=10;picID++){
    
        int picID =10;
      
    
        Mat orgFrame;
        string fileName = "images/"+to_string(picID)+".jpg";
        orgFrame = imread(fileName);
        
        
        if(!orgFrame.data){
            cout <<  "Could not open or find the image" << picID << endl ;
            //continue;
        }
        
        imshow("solved"+to_string(picID), solve(orgFrame));
        
    //}
    
    
    

    waitKey(0);
    return 0;
}
