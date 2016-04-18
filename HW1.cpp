#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Mat showFor(Mat image)
{
	Mat padded;                            //expand input image to optimal size
	int m = getOptimalDFTSize(image.rows);
	int n = getOptimalDFTSize(image.cols); // on the border add zero values
	copyMakeBorder(image, padded, 0, m - image.rows, 0, n - image.cols, BORDER_CONSTANT, Scalar::all(0));

	Mat planes[] = { Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F) };
	Mat complexI;
	merge(planes, 2, complexI);         // Add to the expanded another plane with zeros

	dft(complexI, complexI);            // this way the result may fit in the source matrix

										// compute the magnitude and switch to logarithmic scale
										// => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
	split(complexI, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
	magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
	Mat magI = planes[0];

	magI += Scalar::all(1);                    // switch to logarithmic scale
	log(magI, magI);

	// crop the spectrum, if it has an odd number of rows or columns
	magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));

	// rearrange the quadrants of Fourier image  so that the origin is at the image center
	int cx = magI.cols / 2;
	int cy = magI.rows / 2;

	Mat q0(magI, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
	Mat q1(magI, Rect(cx, 0, cx, cy));  // Top-Right
	Mat q2(magI, Rect(0, cy, cx, cy));  // Bottom-Left
	Mat q3(magI, Rect(cx, cy, cx, cy)); // Bottom-Right

	Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
	q2.copyTo(q1);
	tmp.copyTo(q2);

	normalize(magI, magI, 0, 1, CV_MINMAX); // Transform the matrix with float values into a
											// viewable image form (float between values 0 and 1).
	return magI;
}

int applyfiltering(Mat image, Mat Fourier, int filterType, int setting)
{	
	imshow("Display window", image);
	createTrackbar("D0", "Display window", 0, 255, 0);
	createTrackbar("n", "Display window", 0, 255, 0);
	waitKey(0);
	return 0;
}

int applyFilter(Mat image, Mat fourier)
{
	int filterType;
	cout << "Please choose type of filter: \n1.Low \n2.High \n3.Quit" << endl;
	cin >> filterType;
	if (filterType == 3)
		return 0;
	else if (filterType == 1 || filterType == 2)
	{
		int setting;
		cout << "Please choose type of setting: \n1.Ideal \n2.Butterworth \n3.Gaussian \n4.Quit" << endl;
		cin >> setting;
		if (setting == 1 || setting == 2 || setting == 3)
			applyfiltering(image, fourier, filterType, setting);
		else if (setting == 4)
			return 0;
		else
		{
			cout << " Invalid input" << endl;
			return 0;
		}
	}
	else
	{
		cout << " Invalid input" << endl;
		return 0;
	}
}



int main(int argc, char** argv)
{

	if (argc != 2)
	{
		cout << " Please add name of the image" << endl;
		return -1;
	}
	while (true)
	{
		Mat image;
		Mat fourier;
		image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);   // Read the file

		cout << "Please choose an option: \n1.Show Original Image \n2.Show Power Spectrum \n3.Apply A Filter\n4.Quit" << endl;
		int input;
		cin >> input;

		if (input == 1)
		{
			if (!image.data)                              // Check for invalid input
			{
				cout << "Could not open or find the image" << std::endl;
				return -1;
			}
			namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
			imshow("Display window", image);// Show our image inside it.
			waitKey(0);
			destroyWindow("Display window");

		}
		else if (input == 2)
		{
			fourier = showFor(image);
			imshow("spectrum magnitude", fourier);
			waitKey();
			destroyWindow("spectrum magnitude");
		}
		else if (input == 3)
			applyFilter(image,fourier);
		else if (input == 4)
			return 0;
		else 
			cout << " Invalid input" << endl;
	}
	
	waitKey(0);                                          // Wait for a keystroke in the window
	return 0;
}

