#ifndef _MULTIGRAB_H
#define _MULTIGRAB_H

#include "calibmodel.h"

#define USE_MULTITHREADCAPTURE

#include "multithreadcapture.h"

class MultiGrab {
public:

	CalibModel model;

	//MultiGrab(const char *modelfile="model.bmp") : model(modelfile) {}
	MultiGrab() {};
    ~MultiGrab();

	int init( char *avi_bg_path,
          int capture_width, int capture_height, int v4l_device, int detect_width, int detect_height,
          int desired_capture_fps );
    /// load or train the cache using the given modelfile. if wants_training is true, train;
    /// otherwise try to load and if load files, train
    bool loadOrTrainCache( bool wants_training, const char* modelfile, bool running_on_binoculars = false );

	void allocLightCollector();

	class Cam {
    public:
		CvCapture *cam;
		int width,height;
		int detect_width, detect_height;
		//PlanarObjectDetector detector;
		planar_object_recognizer detector;
		LightCollector *lc;
		MultiThreadCapture *mtc;

		/// stop capturing but leave frame buffers in place
		void shutdownMultiThreadCapture();

        const FTime& getLastProcessedFrameTimestamp() { return detected_frame_timestamp; }
        unsigned int getFrameIndexForTime( const FTime& timestamp ) { return mtc->getFrameIndexForTime( timestamp ); }
		IplImage* getLastProcessedFrame() { return frame; }
		/// fetch the last raw frame + timestamp and put into *frame + timestamp. if *frame is NULL, create.
		bool getLastDrawFrame( IplImage** raw_frame, FTime* timestamp=NULL )
            { return mtc->getLastDrawFrame( raw_frame, timestamp, true /*block*/ ); }

		void setCam(CvCapture *c, int capture_width, int capture_height, int detect_width, int detect_height, int desired_capture_fps );
		bool detect( bool& frame_retrieved, bool &detect_succeeded );

		Cam(CvCapture *c=0, int _width=0, int _height=0, int _detect_width=320, int _detect_height=240, int desired_capture_fps=20 )
		{
		    frame = 0;
			width=0;
			height=0;
			detect_width=_detect_width;
			detect_height=_detect_height;
			cam=0;
			lc=0;
			mtc=0;
			if (c) setCam(c, _width, _height, _detect_width, _detect_height, desired_capture_fps );
			gray=0;
			frame_detectsize=0;
		}
		Cam( const Cam& other ) { assert( false && "copy constructor called, arrgh" ); }
		~Cam();

		private:
            IplImage *frame, *frame_detectsize, *gray;
            FTime detected_frame_timestamp;

	};

	std::vector<Cam *> cams;
	struct Cam *foo;
};

bool add_detected_homography(int n, planar_object_recognizer &detector, CamCalibration &calib);
bool add_detected_homography(int n, planar_object_recognizer &detector, CamAugmentation &a);
IplImage *myQueryFrame(CvCapture *capture);
IplImage *myRetrieveFrame(CvCapture *capture);

#endif
