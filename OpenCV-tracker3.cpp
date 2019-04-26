// tracker3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


/*----------------------------------------------
  * Usage:
  * example_tracking_multitracker <video_name> [algorithm]
  *
  * example:
  * example_tracking_multitracker Bolt/img/%04d.jpg
  * example_tracking_multitracker faceocc2.webm KCF
  *--------------------------------------------------*/
 
#include <opencv2/core/utility.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include "D:/Projects/openCV-extraModules/opencv_contrib-master/modules/tracking/samples/samples_utility.hpp"
#include <time.h>

using namespace std;
using namespace cv;

const int MAX_MULTITRACKERS = 20;

// Get a unique file name, format is TrackYYYYMMDD-HH:mm:ss.txt
const std::string createUniqueFilenameBasedOnDateTime(string fileName) {
    time_t rawtime = time(0);
    tm timeinfo;
    char timestamp[80];
    errno_t result = localtime_s(&timeinfo, &rawtime);
	sprintf_s(timestamp, "%04d%02d%02d%02d%02d%02d", 1900+timeinfo.tm_year, 1+timeinfo.tm_mon, timeinfo.tm_mday,
			timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

	fileName = fileName.substr(0, fileName.size()-sizeof(".mp4")+1); // get rid of the .mp4
	fileName = fileName + "-" + timestamp + ".txt";
    return fileName;
}

int main( int argc, char** argv ){
  // show help
  if(argc<2){
    cout<<
      " Usage: tracker3_multitracker <video_name> [algorithm]\n"
      " examples:\n"
      " tracker3_multitracker Bolt/img/%04d.jpg\n"
      " tracker3_multitracker faceocc2.webm MEDIAN_FLOW\n"
      " algorithm can be: MIL(slow), BOOSTING(good), MEDIAN_FLOW(ok), TLD(slow), KCF(bad), GOTURN(crash), MOSSE(vido fast but not tracking).\n"
	  << endl;
    return 0;
  }

  // set the default tracking algorithm
  string trackingAlg = "BOOSTING";

  // set the tracking algorithm from parameter
  if(argc>2)
    trackingAlg = argv[2];

  // create the tracker
  MultiTracker* trackers[MAX_MULTITRACKERS];

  // container of the tracked objects
  vector<Rect2d> objects;

  // set input video
  string video = argv[1];
  VideoCapture cap(video);

  Mat frame;

  // get bounding box
  cap >> frame;
  vector<Rect> ROIs;
  selectROIs("tracker",frame,ROIs);

  //quit when the tracked object(s) is not provided
  if(ROIs.size()<1)
    return 0;

  // initialize the tracker
  vector<Ptr<Tracker> >* algorithms[MAX_MULTITRACKERS];
  int Cnt_MultiTracker=0;
  algorithms[Cnt_MultiTracker] = new vector<Ptr<Tracker> >;
  for (size_t i = 0; i < ROIs.size(); i++)
  {
      algorithms[Cnt_MultiTracker]->push_back(createTrackerByName(trackingAlg));
      objects.push_back(ROIs[i]);
  }

  trackers[Cnt_MultiTracker] = new MultiTracker;
  trackers[Cnt_MultiTracker]->add(*algorithms[Cnt_MultiTracker],frame,objects);

  long frame_num=0;
  // do the tracking
  printf("Start the tracking process, press ESC to quit.\n");

  char rect_label[8];
  uint obj_num;
  Rect2d object;
  ofstream ofs;
  string filename = createUniqueFilenameBasedOnDateTime(video);
  ofs.open (filename.c_str(), std::ofstream::out);

  ofs<<"Target Team Frame#(time) X Y"<<endl;
  for ( ;; ){
    // get frame from the video
    cap >> frame;
	frame_num++;

    // stop the program if no more images
    if(frame.rows==0 || frame.cols==0)
      break;

    //update the tracking result
    trackers[Cnt_MultiTracker]->update(frame);

    // draw the tracked objects and output to file
	objects = trackers[Cnt_MultiTracker]->getObjects();
    for(unsigned i=0;i<objects.size();i++) {
      object = objects[i];
      rectangle( frame, object, Scalar( 255, 0, 0 ), 2, 1 );
	  sprintf_s(rect_label, "%d", i+1);
      putText( frame, rect_label, Point(object.x+object.width/2, object.y),
                         cv::FONT_HERSHEY_SIMPLEX, 0.5, Scalar( 0, 0, 255 ),
                         2, LINE_8, false );

	  ofs<<i+1<<", "<<(i<7? 1:2)<<", "<<frame_num<<", "<<object.x+object.width/2<<", "<<object.y+object.height/2<<endl;
	}

    // show image with the tracked object
    imshow("tracker",frame);

    int keyPressed;
    keyPressed=waitKey(1);
    //quit on ESC button
    if(keyPressed==27)break;
	//adjust some tracker windows on Enter
	if(keyPressed==13) {
//		delete algorithms[Cnt_MultiTracker];
		delete trackers[Cnt_MultiTracker];
		Cnt_MultiTracker++;
		if(Cnt_MultiTracker==MAX_MULTITRACKERS) {
			cout<<"No more multitracker available"<<endl;
			continue;
		}

		while(1) {
			cout<<"Enter the object's number (Enter -1 twice to start tracking): ";
			cin>>obj_num;
			while(obj_num<=0 || obj_num>objects.size()) {
			  cout<<"Object's number entered is out of range ["<<1<<", "<<objects.size()<<"]"<<endl;
			  cout<<"Enter the object's number (Enter -1 twice to start tracking): ";
			  cin>>obj_num;
			  if(obj_num==-1) break;
			}
			if(obj_num==-1) break;

			object = selectROI("tracker",frame);
			objects[obj_num-1]=object;
			cout<<"Object #"<<obj_num<<" updated"<<endl;
		}

		algorithms[Cnt_MultiTracker] = new vector<Ptr<Tracker> >;
		trackers[Cnt_MultiTracker] = new MultiTracker;
		for (size_t i = 0; i < ROIs.size(); i++)
		      algorithms[Cnt_MultiTracker]->push_back(createTrackerByName(trackingAlg));
		trackers[Cnt_MultiTracker]->add(*algorithms[Cnt_MultiTracker],frame,objects);
	}
  }
  ofs.close();
}
