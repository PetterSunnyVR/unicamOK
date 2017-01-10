// CVBTest.cpp : Defines the entry point for the console application.
//

/************************************************************************
STEMMER IMAGING GmbH
------------------------------------------------------------------------
Program  :  VCGenICamGrabConsoleExample.exe
Author   :
Date     :  12/2009
Purpose  :  C++ Console example which shows how easy it is to Grab Images
with the CVB Grab2 Interface and the GenICam vin driver.

Revision :  1.1
Updates  :  07/2010 CKS Adapted to CVB 2010
02/2011 SDo Added -scan parameter
************************************************************************/

#include <Windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <iCVCImg.h>
#include <iCVCDriver.h>
#include <iCVCUtilities.h>
#include <iCVGenApi.h>
#include <CVCError.h>
#include "opencv2\core\core.hpp"
#include "opencv2\opencv.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv\highgui.h>
#include "main.h"

using namespace std;
using namespace cv;

int ocv_depth(cvbdatatype_t cvbDt);
int grabMat();
Mat returnMat();
// constant for path length
static const size_t DRIVERPATHSIZE = 256;
// command line argument to scan for new cameras
static const string SCAN("-scan");
Mat m;
// see implementation below on how to access a camera via GenICam GenApi
void genicam_access(IMG hCamera);
// helper to scan for new cameras on load
void scan_for_cameras();




int ocv_depth(cvbdatatype_t cvbDt)
{
	// map compatible cvb data type descriptors to OpenCV data types
	switch (cvbDt & 0xFF)
	{
	case 8:
		return IsSignedDatatype(cvbDt) ? IPL_DEPTH_8S : IPL_DEPTH_8U;
	case 16:
		return IsSignedDatatype(cvbDt) ? IPL_DEPTH_16S : IPL_DEPTH_16U;
	case 32:
		return IsFloatDatatype(cvbDt) ? IPL_DEPTH_32F : IPL_DEPTH_32S;
	case 64:
		return IsFloatDatatype(cvbDt) ? IPL_DEPTH_64F : 0;
	default:
		return 0;
	}
}

// entry function
int grabMat()
{

	//bool doScan = true;
	// scan for new cameras?
	//if (doScan)
	//	scan_for_cameras();

	// load the first camera
	char driverPath[DRIVERPATHSIZE] = { 0 };
	TranslateFileName("%CVB%\\Drivers\\GenICam.vin", driverPath, DRIVERPATHSIZE);
	IMG hCamera = NULL;
	cvbbool_t success = LoadImageFile(driverPath, hCamera);
	if (!success)
	{
		cout << "Error loading \"" << driverPath << "\" driver!" << endl;
		cout << "Probably no cameras ";
		if (true)
			cout << "were found during discovery!" << endl;
		else
			cout << "are configured/reachable!" << endl;
		return 1;
	}
	cout << "Load \"" << driverPath << "\" successful." << endl;

	// access camera config
	if (CanNodeMapHandle(hCamera))
	{
		genicam_access(hCamera);
	}

	// start grab with ring buffer
	cout << "Acquire 10 images" << endl;
	cvbres_t result = G2Grab(hCamera);
	if (result >= 0)
	{
		// acquire 10 images
		for (cvbval_t i = 0; i < 1; ++i)
		{
			// wait for next image to be acquired
			// (returns immediately if unprocessed image are in the ring buffer)
			result = G2Wait(hCamera);
			if (result < 0)
			{
				cout << setw(4) << i << " Error with G2Wait: " << CVC_ERROR_FROM_HRES(result) << endl;
			}
			else
			{
				// do image processing
				//void *pBase = NULL;
				//intptr_t xInc = 0;
				//intptr_t yInc = 0;
				//GetLinearAccess(hCamera, 0, &pBase, &xInc, &yInc);
				//cout << setw(4) << i << " Acquired image @" << hex << pBase << endl;

				CvSize size;
				size.width = ImageWidth(hCamera);
				size.height = ImageHeight(hCamera);

				IplImage* retval = cvCreateImageHeader(size, ocv_depth(ImageDatatype(hCamera, 0)), ImageDimension(hCamera));

				if (retval == nullptr)
					cvSaveImage("E:/foo.png", retval);
				void* ppixels = nullptr;
				intptr_t xInc = 0;
				intptr_t yInc = 0;
				GetLinearAccess(hCamera, 0, &ppixels, &xInc, &yInc);
				cvSetData(retval, ppixels, yInc);
				cvSaveImage("E:/foo1.png", retval);
				//true - copy data
				m = cv::cvarrToMat(retval,true);
				cout << "Success! Mat size 0.5 ? " << m.size() << endl;
				cout << "Mat addr 0.5? " << &m << endl;
				cout << "Mat addr 0.5? " << returnMat().size() << endl;
				return 1;
			}
		}

		std::cout << "Stop acquisition" << endl;
		// stop the grab (kill = false: wait for ongoing frame acquisition to stop)
		result = G2Freeze(hCamera, true);
	}
	else
	{
		cout << "Error starting acquisition!" << endl;
	}

	// free camera
	cout << "Free camera" << endl;
	ReleaseObject(hCamera);

	return 0;
}

// access a feature via CVGenApi
void genicam_access(IMG hCamera)
{
	cout << "Read camera image width: ";

	NODEMAP hNodeMap = NULL;
	cvbres_t result = NMHGetNodeMap(hCamera, hNodeMap);
	if (result >= 0)
	{
		// get width feature node
		NODE hWidth = NULL;
		result = NMGetNode(hNodeMap, "Width", hWidth);
		if (result >= 0)
		{
			// value camera dependent
			cvbint64_t widthValue = 0;
			result = NGetAsInteger(hWidth, widthValue);
			// set values via NSetAsInteger(hWidht, widthValue);
			if (result >= 0)
			{
				cout << widthValue << endl;
			}
			else
			{
				cout << "Node value error: " << CVC_ERROR_FROM_HRES(result) << endl;
			}

			ReleaseObject(hWidth);
		}
		else
		{
			cout << "Node error: " << CVC_ERROR_FROM_HRES(result) << endl;
		}
		ReleaseObject(hNodeMap);
	}
	else
	{
		cout << "NodeMap error: " << CVC_ERROR_FROM_HRES(result) << endl;
	}
};

// sets the ini-file entry which populates the ini with currently attached 
// cameras
// (the flag is auto-reset when the driver was loaded)
void scan_for_cameras()
{
	char iniPath[DRIVERPATHSIZE] = { 0 };
	TranslateFileName("%CVBDATA%\\Drivers\\GenICam.ini", iniPath, DRIVERPATHSIZE);

	cout << "Configure driver to scan for cameras: ";
	BOOL result = WritePrivateProfileStringA("SYSTEM", "CreateAutoIni", "1", iniPath);
	cout << (result ? "successful" : "error") << endl << endl;
};

Mat returnMat() {
	return m;
};

JNIEXPORT void JNICALL Java_main_FindFeatures
(JNIEnv *, jobject, jlong addrGray) {
	Mat* mGr = (Mat*)addrGray;

	
	cout << "Running app " << grabMat() << endl;
	cout << "Mat addr 1? " << &m << endl;
	//ch
	*mGr = m;
	cout << "Success! Mat size? " << m.size() << endl;
	//ch
	cout << "Success! Mat size? " << mGr->size() << endl;
};