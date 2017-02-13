#include <vector>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/video/tracking.hpp>
#define MAX_CONTOUR_SIZE 2000

using namespace cv;
using namespace std;

RNG rng(12345);

/**
 * Helper function to display text in the center of a contour
 */
void setLabel(cv::Mat& im, const std::string label, std::vector<cv::Point>& contour)
{
	int fontface = cv::FONT_HERSHEY_SIMPLEX;
	double scale = 0.4;
	int thickness = 1;
	int baseline = 0;

	cv::Size text = cv::getTextSize(label, fontface, scale, thickness, &baseline);
	cv::Rect r = cv::boundingRect(contour);

	cv::Point pt(r.x + ((r.width - text.width) / 2), r.y + ((r.height + text.height) / 2));
	cv::rectangle(im, pt + cv::Point(0, baseline), pt + cv::Point(text.width, -text.height), CV_RGB(255,255,255), CV_FILLED);
	cv::putText(im, label, pt, fontface, scale, CV_RGB(0,0,0), thickness, 8);
}

int imageProcessing(Mat imgInput, int low_hue, int high_hue, int minimum_area, int *detected_center_x, int *detected_center_y, double *contour_area, double* detected_radius);

	int iLowH 	= 0;	// yellow = 15, red = 166 , blue = 92
	int iHighH 	= 179;	// yellow = 29, red = 179 , blue = 114
	
	int iLowS	= 90; // lower this to allow more noise, 80 is normal
	int iHighS 	= 255;
	
	int iLowV 	= 0; // lower this to allow more noise
	int iHighV 	= 255;
	
	

int main( int argc, char** argv ){
	
	//VideoCapture cap("test.avi"); //capture the video from webcam
	VideoCapture cap(3); //capture the video from webcam
	
	if (!cap.isOpened()){  // if not success, exit program
		cout << "Cannot open the web cam" << endl;
		return -1;
	}
	
	namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"
	//Create trackbars in "Control" window
	createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	createTrackbar("HighH", "Control", &iHighH, 179);
	
	createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	createTrackbar("HighS", "Control", &iHighS, 255);
	
	createTrackbar("LowV", "Control", &iLowV, 255);//Value (0 - 255)
	createTrackbar("HighV", "Control", &iHighV, 255);
	
	
	namedWindow("Image", CV_WINDOW_NORMAL); //create a window called "Thresholded Image"
		
	while (true){
		
		Mat imageRead;//	 = imread(argv[1], CV_LOAD_IMAGE_UNCHANGED);
	
		bool bSuccess = cap.read(imageRead); // read a new frame from video
		if (!bSuccess){ //if not success, break loop
			
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}
		
		int center_buoy_x	= 0;
		int center_buoy_y 	= 0;
		double buoy_area	= 0;
		double radius_buoy	= 0;
		
		int buoy_number = imageProcessing(imageRead, 15, 29,MAX_CONTOUR_SIZE, &center_buoy_x, &center_buoy_y, &buoy_area, &radius_buoy);
		
		cout << "######## Posisi Yellow #########" << endl;
		cout << "Detected Buoy : " << buoy_number << endl;
		cout << "Posisi X  : " << center_buoy_x << endl;
		cout << "Posisi Y  : " << center_buoy_y << endl;
		cout << "Bouy Size : " << buoy_area << endl;
		cout << endl;
		cout << endl;
		
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		Point2f circle_center;
		circle_center.x = center_buoy_x;
		circle_center.y = center_buoy_y;
		circle( imageRead, circle_center, radius_buoy, color, 4, 8, 0 );
		
		imshow("Image", imageRead );
		
		if (waitKey(30) == 27){ //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
			
			cout << "esc key is pressed by user" << endl;
			break;
		}
		
	}
	
	return 0;
}

int imageProcessing(Mat imgInput, int low_hue, int high_hue, int minimum_area, int *detected_center_x, int *detected_center_y, double *contour_area, double* detected_radius){
	
	Mat imgOriginal = imgInput.clone();
		
	GaussianBlur( imgOriginal, imgOriginal, Size( 5, 5 ), 0, 0 );
	
	Mat imgHSV;
	cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
	
	Mat imgThresholded;
	
	inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image
	
	
	cv::imshow("imgThresholded", imgThresholded);
	
	cv::Mat bw;
	cv::Canny(imgThresholded, bw, 0, 50, 5);
	
	cv::imshow("bw", bw);
	
	// Find contours
	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(bw.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	std::vector<cv::Point> approx;
	cv::Mat dst = imgInput.clone();
	
	Point2f center;
	float radius;
	int largest_bouy_area = 0;
	int largest_bouy_id = 0;
	int number_of_detected_shape = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		// Approximate contour with accuracy proportional
		// to the contour perimeter
		cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);

		if (approx.size() == 3 && std::fabs(cv::contourArea(contours[i])) > minimum_area && cv::isContourConvex(approx))
		{
			double a = contourArea( contours[i],false);  //  Find the area of contour
			if(a > largest_bouy_area && a > minimum_area){
				largest_bouy_area = a;
				largest_bouy_id = i;                //Store the index of largest contour
				minEnclosingCircle( contours[i], center, radius);
			}
			if(largest_bouy_area > minimum_area){
				number_of_detected_shape++;
			}
			cout << "Triangle Circumference : " << cv::arcLength(contours[i], true) << endl;
			cout << "Triangle Area : " << cv::contourArea(contours[i]) << endl;
			setLabel(dst, "TRIANGLE", contours[i]);    // Triangles			
		}
		
		else if (approx.size() == 12 && std::fabs(cv::contourArea(contours[i])) > MAX_CONTOUR_SIZE)
		{
			cout << "Cross Circumference : " << cv::arcLength(contours[i], true) << endl;
			cout << "Cross Area : " << cv::contourArea(contours[i]) << endl;
			setLabel(dst, "CROSS", contours[i]);    // Triangles
		}

	}

	if(largest_bouy_area > minimum_area){
		
		*detected_center_x	=	(int)center.x;
		*detected_center_y	=	(int)center.y;
		*contour_area		=	largest_bouy_area;
		*detected_radius	=	radius;
	}

	cv::imshow("dst", dst);
	
	return number_of_detected_shape;
}

