#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <fstream>
#include <string>
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/tss.hpp>
#include <boost/timer/timer.hpp>

using namespace cv;
using namespace std;
using namespace boost;

/** Function Headers */
void numberOfFaces(string filePath);
void drawFaceElipse(Mat faceImage, std::vector<Rect> faces);


String face_cascade_name = "haarcascade_frontalface_alt.xml";
//CascadeClassifier face_cascade;
string filePrefix = "C:\\Users\\bpaulin\\Downloads\\mirflickr25k\\mirflickr\\im";
string fileSuffix = ".jpg";

atomic<int> totalFaces(0);
static boost::thread_specific_ptr< CascadeClassifier> instance;

int main(int argc, char** argv)
{
	boost::asio::io_service ioService;
	boost::asio::io_service::work work(ioService);
	boost::thread_group threadpool;
	int numThreads = 2;
	for (int i = 0; i < numThreads; i++)
	{
		threadpool.create_thread(
			boost::bind(&boost::asio::io_service::run, &ioService)
			);
	}
	
	ifstream infile("people.txt");
	string peopleLine;

	timer::cpu_timer timer;
	while (infile >> peopleLine)
	{
		string filePath = filePrefix + peopleLine + fileSuffix;

		if (!boost::filesystem::exists(filePath))
		{
			cout << "Can't find my file: " << filePath << endl;
		}
		else
		{	
			//numberOfFaces(filePath);

			ioService.post(boost::bind(&numberOfFaces, filePath));
		}
		
	}
	ioService.stop();
	threadpool.join_all();
	cout << timer.format() << '\n';

	cout << "Total Number of Faces is: "  << totalFaces << endl;
	
	return 0;
}

void numberOfFaces(string filePath)
{ 
	if (!instance.get()) {
		CascadeClassifier* face_cascade = new CascadeClassifier();
		face_cascade->load(face_cascade_name);
		instance.reset(face_cascade);
	}

	Mat faceImage = imread(filePath, IMREAD_COLOR);
	if (faceImage.empty()) // Check for invalid input
	{
		cout << "Could not open or find the image" << endl;
		return;
	}

	Mat frame_gray;
	std::vector<Rect> faces;
	cvtColor(faceImage, frame_gray, CV_BGR2GRAY);
	equalizeHist(frame_gray, frame_gray);

	instance->detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));
	int numFaces = faces.size();
	//cout << "Found " << numFaces << " faces in image: " << filePath << endl;
	totalFaces.fetch_add(numFaces, boost::memory_order_seq_cst);
	//cout << "Total: " << totalFaces << endl;
	/*if (numFaces > 0)
	{
		drawFaceElipse(faceImage, faces);
	}*/
}

void drawFaceElipse(Mat faceImage, std::vector<Rect> faces)
{
	
		cout << "Face Found!" << endl;

		Point center(faces[0].x + faces[0].width*0.5, faces[0].y + faces[0].height*0.5);
		Size centerSize(faces[0].width*0.5, faces[0].height*0.5);
		ellipse(faceImage, center, centerSize, 0, 0, 360, Scalar(255, 0, 255), 4, 8, 0);
	


	namedWindow("Display window", WINDOW_AUTOSIZE); // Create a window for display.
	imshow("Display window", faceImage); // Show our image inside it.

	waitKey(0); // Wait for a keystroke in the window
}