//#include <opencv2\opencv.hpp>
//#include <iostream>
//#include <string>
//using namespace cv;
//using namespace std;
//int main()
//{
//    Mat img = imread("pp.jpg");
//    if(img.empty())
//    {
//        cout<<"error";
//        return -1;
//    }
//    imshow("xx的靓照",img);
//    waitKey();
//  cout<<"wangzhe"<<endl;
//  
//    return 0;
//}

//#include <opencv2\core\utility.hpp>
//#include "opencv2\video.hpp"
//#include "opencv2/imgcodecs.hpp"
//#include "opencv2/highgui.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdio.h>
#include <iostream>
#include <fstream>
using namespace cv;
using namespace std;

inline bool isFlowCorrect(Point2f u)
{
    return !cvIsNaN(u.x) && !cvIsNaN(u.y) && fabs(u.x) < 1e9 && fabs(u.y) < 1e9;
}

static Vec3b computeColor(float fx, float fy)
{
    static bool first = true;

    // relative lengths of color transitions:
    // these are chosen based on perceptual similarity
    // (e.g. one can distinguish more shades between red and yellow
    //  than between yellow and green)
    const int RY = 15;
    const int YG = 6;
    const int GC = 4;
    const int CB = 11;
    const int BM = 13;
    const int MR = 6;
    const int NCOLS = RY + YG + GC + CB + BM + MR;
    static Vec3i colorWheel[NCOLS];

    if (first)
    {
        int k = 0;

        for (int i = 0; i < RY; ++i, ++k)
            colorWheel[k] = Vec3i(255, 255 * i / RY, 0);

        for (int i = 0; i < YG; ++i, ++k)
            colorWheel[k] = Vec3i(255 - 255 * i / YG, 255, 0);

        for (int i = 0; i < GC; ++i, ++k)
            colorWheel[k] = Vec3i(0, 255, 255 * i / GC);

        for (int i = 0; i < CB; ++i, ++k)
            colorWheel[k] = Vec3i(0, 255 - 255 * i / CB, 255);

        for (int i = 0; i < BM; ++i, ++k)
            colorWheel[k] = Vec3i(255 * i / BM, 0, 255);

        for (int i = 0; i < MR; ++i, ++k)
            colorWheel[k] = Vec3i(255, 0, 255 - 255 * i / MR);

        first = false;
    }

    const float rad = sqrt(fx * fx + fy * fy);
    const float a = atan2(-fy, -fx) / (float)CV_PI;

    const float fk = (a + 1.0f) / 2.0f * (NCOLS - 1);
    const int k0 = static_cast<int>(fk);
    const int k1 = (k0 + 1) % NCOLS;
    const float f = fk - k0;

    Vec3b pix;

    for (int b = 0; b < 3; b++)
    {
        const float col0 = colorWheel[k0][b] / 255.f;
        const float col1 = colorWheel[k1][b] / 255.f;

        float col = (1 - f) * col0 + f * col1;

        if (rad <= 1)
            col = 1 - rad * (1 - col); // increase saturation with radius
        else
            col *= .75; // out of range

        pix[2 - b] = static_cast<uchar>(255.f * col);
    }

    return pix;
}

static void drawOpticalFlow(const Mat_<Point2f>& flow, Mat& dst, float maxmotion = -1)
{
    dst.create(flow.size(), CV_8UC3);
    dst.setTo(Scalar::all(0));

    // determine motion range:
    float maxrad = maxmotion;

    if (maxmotion <= 0)
    {
        maxrad = 1;
        for (int y = 0; y < flow.rows; ++y)
        {
            for (int x = 0; x < flow.cols; ++x)
            {
                Point2f u = flow(y, x);

                if (!isFlowCorrect(u))
                    continue;

                maxrad = max(maxrad, sqrt(u.x * u.x + u.y * u.y));
            }
        }
    }

    for (int y = 0; y < flow.rows; ++y)
    {
        for (int x = 0; x < flow.cols; ++x)
        {
            Point2f u = flow(y, x);

            if (isFlowCorrect(u))
                dst.at<Vec3b>(y, x) = computeColor(u.x / maxrad, u.y / maxrad);
        }
    }
}

static void drawOpticalFlowGray(const Mat_<Point2f>& flow, Mat& dst, float maxmotion = -1)
{
    dst.create(flow.size(), CV_8UC1);
    dst.setTo(Scalar::all(0));

    // determine motion range:
    float maxrad = maxmotion;

    if (maxmotion <= 0)
    {
        maxrad = 1;
        for (int y = 0; y < flow.rows; ++y)
        {
            for (int x = 0; x < flow.cols; ++x)
            {
                Point2f u = flow(y, x);

                if (!isFlowCorrect(u))
                    continue;

                maxrad = max(maxrad, sqrt(u.x * u.x + u.y * u.y));
            }
        }
    }
    //#define CAST(v, L, H) ((v) > (H) ? 255 : (v) < (L) ? 0 : cvRound(255*((v) - (L))/((H)-(L))))
    //for (int i = 0; i < flow_x.rows; ++i) {
    //  for (int j = 0; j < flow_y.cols; ++j) {
    //      float x = flow_x.at<float>(i,j);
    //      float y = flow_y.at<float>(i,j);
    //      img_x.at<uchar>(i,j) = CAST(x, lowerBound, higherBound);
    //      img_y.at<uchar>(i,j) = CAST(y, lowerBound, higherBound);
    //  }
    //}
    //#undef CAST

   /* for (int y = 0; y < flow.rows; ++y)
    {
        for (int x = 0; x < flow.cols; ++x)
        {
            Point2f u = flow(y, x);

            if (isFlowCorrect(u))
                dst.at<Vec3b>(y, x) = computeColor(u.x / maxrad, u.y / maxrad);
        }
    }*/
}

// binary file format for flow data specified here:
// http://vision.middlebury.edu/flow/data/
static void writeOpticalFlowToFile(const Mat_<Point2f>& flow, const string& fileName)
{
    static const char FLO_TAG_STRING[] = "PIEH";

    ofstream file(fileName.c_str(), ios_base::binary);

    file << FLO_TAG_STRING;

    file.write((const char*) &flow.cols, sizeof(int));
    file.write((const char*) &flow.rows, sizeof(int));

    for (int i = 0; i < flow.rows; ++i)
    {
        for (int j = 0; j < flow.cols; ++j)
        {
            const Point2f u = flow(i, j);

            file.write((const char*) &u.x, sizeof(float));
            file.write((const char*) &u.y, sizeof(float));
        }
    }
}
static void convertFlowToImage(const Mat &flow_x, const Mat &flow_y, Mat &img_x, Mat &img_y, 
       double lowerBound, double higherBound) {
    #define CAST(v, L, H) ((v) > (H) ? 255 : (v) < (L) ? 0 : cvRound(255*((v) - (L))/((H)-(L))))
    for (int i = 0; i < flow_x.rows; ++i) {
        for (int j = 0; j < flow_y.cols; ++j) {
            float x = flow_x.at<float>(i,j);
            float y = flow_y.at<float>(i,j);
            img_x.at<uchar>(i,j) = CAST(x, lowerBound, higherBound);
            img_y.at<uchar>(i,j) = CAST(y, lowerBound, higherBound);
        }
    }
    #undef CAST
}
int main(int argc, char** argv){
    // IO operation
    const char* keys =
        {
            "{ f  | vidFile      | ex2.avi | filename of video }"
            "{ x  | xFlowFile    | flow_x | filename of flow x component }"
            "{ y  | yFlowFile    | flow_y | filename of flow x component }"
            "{ i  | imgFile      | flow_i | filename of flow image}"
            "{ b  | bound | 15 | specify the maximum of optical flow}"
        };

    CommandLineParser cmd(argc, argv, keys);
    string vidFile = cmd.get<string>("vidFile");
    string xFlowFile = cmd.get<string>("xFlowFile");
    string yFlowFile = cmd.get<string>("yFlowFile");
    string imgFile = cmd.get<string>("imgFile");
    int bound = cmd.get<int>("bound");


    VideoCapture capture(vidFile);
    if(!capture.isOpened()) {
        printf("Could not initialize capturing..\n");
        return -1;
    }

    int frame_num = 0;
    Mat image, prev_image, prev_grey, grey, frame, flow, cflow;

    while(true) {
        capture >> frame;
        if(frame.empty())
            break;
        if(frame_num == 0) {
            image.create(frame.size(), CV_8UC3);
            grey.create(frame.size(), CV_8UC1);
            prev_image.create(frame.size(), CV_8UC3);
            prev_grey.create(frame.size(), CV_8UC1);

            frame.copyTo(prev_image);
            cvtColor(prev_image, prev_grey, CV_BGR2GRAY);

            frame_num++;
            continue;
        }
    frame.copyTo(image);
        cvtColor(image, grey, CV_BGR2GRAY);



    Mat_<Point2f> flow;
    Ptr<DenseOpticalFlow> tvl1 = createOptFlow_DualTVL1();

    const double start = (double)getTickCount();
    tvl1->calc(prev_grey, grey, flow);
   
    Mat flows[2];
    split(flow,flows);

    Mat out;
    drawOpticalFlow(flow, out);
    Mat imgX(flows[0].size(),CV_8UC1);
    Mat imgY(flows[0].size(),CV_8UC1);
    convertFlowToImage(flows[0],flows[1], imgX, imgY, -15, 15);
   
    char tmp[200];
        sprintf(tmp,"_%05d.jpg",int(frame_num));
        
        Mat imgX_, imgY_, image_;
        cv::resize(imgX,imgX_,cv::Size(256,256));
        cv::resize(imgY,imgY_,cv::Size(256,256));
        cv::resize(image,image_,cv::Size(256,256));
        
        imwrite(xFlowFile + tmp,imgX_);
        imwrite(yFlowFile + tmp,imgY_);
        imwrite(imgFile + tmp, image_);

        std::swap(prev_grey, grey);
        std::swap(prev_image, image);
        frame_num = frame_num + 1;
    }
    return 0;
}

