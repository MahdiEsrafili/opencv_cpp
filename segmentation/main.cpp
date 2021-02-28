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
    result = background_remove(img, background, lightMethod);
    imshow("original image", img);
    imshow("background removed", result);
    waitKey(0);
    destroyAllWindows();
    return 0;
}