#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <memory>
using namespace std;

#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
const char* keys =
{
	"{help h usage ? | | print this message}"
	"{@image || Image to process}"
	"{@lightPattern || Image light pattern to apply to image input}"
	"{lightMethod | 1 | Method to remove backgroun light, 0 differenec, 1 div, 2 no light removal' }"
	"{segMethod | 1 | Method to segment: 1 connected Components, 2 connectec components with stats, 3 find Contours }"
};

void denoise(Mat in_image, Mat* out_image){
    medianBlur(in_image, *out_image, 5 );
}

Mat background_remove(Mat in_img, Mat background_pattern, int method){
    // method : 0 subtract, 1 divide
    Mat result;
    if (method==1){
        Mat img32, background32, aux; 
        in_img.convertTo(img32, CV_32F);
        background_pattern.convertTo(background32, CV_32F);
        aux = 1 - (img32/background32);
        aux.convertTo(result, CV_8U, 255);
    }
    else{
        result = background_pattern - in_img;
    }
    return result;
}

static Scalar randomColor( RNG& rng )
{
	auto icolor = (unsigned) rng;
	return Scalar( icolor&255, (icolor>>8)&255, (icolor>>16)&255 );
}

void connected_objects(Mat image){
    Mat labels;
    auto num_objects = connectedComponents(image, labels);
    cout << "connected objects= " << num_objects << endl;
    Mat output = Mat::zeros(image.rows, image.cols, CV_8UC3);
    RNG rng(0xFFFFFFFF);
    for (auto i=1; i<num_objects; i++){
        Mat mask = labels==i;
        output.setTo(randomColor(rng), mask);
    }
    imshow("objects detected", output);
}

void connected_objects_advanced(Mat image){
    Mat labels, stats, centroids;
    auto num_objects = connectedComponentsWithStats(image, labels, stats, centroids);
    Mat output = Mat::zeros(image.rows, image.cols, CV_8UC3);
    RNG rng(0xFFFFFFFF);
    int valid_objects = 0;
    for (auto i=1; i<num_objects; i++){
        if (stats.at<int>(i, CC_STAT_AREA) < 50)
            continue;
        valid_objects++ ;
        Mat mask = labels==i;
        output.setTo(randomColor(rng), mask);
        stringstream ss;
        ss << "area: " << stats.at<int>(i, CC_STAT_AREA);
        putText(output,
            ss.str(),
            centroids.at<Point2d>(i),
            FONT_HERSHEY_SIMPLEX,
            0.4,
            Scalar(255, 255, 255));
    }
    cout << "objects detected = "<< valid_objects << endl;
    imshow("objects detected", output);
}

void find_contours(Mat image){
    vector<vector<Point>> contours;
    findContours(image, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    Mat output = Mat::zeros(image.rows, image.cols, CV_8UC3);
    RNG rng(0xFFFFFFFF);
    int num_contours = 0 ;
    for (auto i=0; i<contours.size(); i++){
        if(contours[i].size() < 100)
            continue;
        num_contours++;
        drawContours(output, contours, i, randomColor(rng));
        imshow("find contours", output);
    }
    cout << "found contours= " << num_contours<< endl;
}

int main(int argc, const char** argv){
    CommandLineParser parser(argc, argv, keys);
    parser.about("Image Segmentation");
    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }
    
    String img_file = parser.get<String>(0);
    String background_file = parser.get<String>(1);
    auto lightMethod = parser.get<int>("lightMethod");
    Mat img = imread(img_file, 0);
    Mat background = imread(background_file, 0);
    if ((img.data==NULL) or (background.data==NULL)){
        cout<< "no image or background pattern" <<endl;
        return 0;
    }
    Mat img_denoised;
    denoise(img, &img_denoised);
    Mat result;
    imshow("original image", img);
    result = background_remove(img, background, lightMethod);
    connected_objects_advanced(result);
    find_contours(result);
    waitKey(0);
    destroyAllWindows();
    return 0;
}