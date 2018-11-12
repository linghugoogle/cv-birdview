#ifndef BIRDVIEW_HPP
#define BIRDVIEW_HPP

#include "time.h"
#include <opencv2/opencv.hpp>

class Birdview
{

public:
    enum modes
    {
        INPUT,
        SMOOTH,
        CANNY,
        CONTOURS
    };

    explicit Birdview();
    ~Birdview();

    void load(std::string path);
    void preprocess();
    void contours();
    void boundingbox();
    void viewpoints();
    void debug(const modes &level);

private:
    static const unsigned int SCALE = 5;
    cv::RNG m_rng;
    cv::Mat m_imgInput;
    cv::Mat m_imgGrey;
    cv::Mat m_imgSmooth;
    cv::Mat m_imgCanny;
    cv::Mat m_imgContours;
    cv::Mat m_imgOutput;
    std::vector<std::vector<cv::Point> > m_contours;
    std::vector<cv::Vec4i> m_hierarchy;
    std::vector<cv::Point> m_contourApprox;
};

#endif // BIRDVIEW_HPP