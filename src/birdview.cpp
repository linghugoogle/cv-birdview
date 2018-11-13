#include "birdview.hpp"

Birdview::Birdview()
{
    m_rng = cv::RNG(time(NULL));
}

Birdview::~Birdview()
{
}

void Birdview::load(std::string path)
{
    m_imgInput = cv::imread(path);
}

void Birdview::preprocess()
{
    cv::resize(m_imgInput, m_imgInput, cv::Size(m_imgInput.cols / SCALE, m_imgInput.rows / SCALE));
    cv::cvtColor(m_imgInput, m_imgGrey, cv::COLOR_BGR2GRAY);
    cv::bilateralFilter(m_imgGrey, m_imgSmooth, 11, 17, 17);
    cv::Canny(m_imgSmooth, m_imgCanny, 30, 200);
    m_imgInputClone = m_imgInput.clone();
}

void Birdview::contours()
{
    m_imgContours = cv::Mat::zeros(m_imgCanny.size(), CV_8UC3);
    cv::findContours(m_imgCanny, m_contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    for(int i = 0; i < static_cast<int>(m_contours.size()); i++)
    {
        cv::Scalar color = cv::Scalar(m_rng(255), m_rng(255), m_rng(255));
        cv::drawContours(m_imgContours, m_contours, i, color, 2, 8, m_hierarchy, 0, cv::Point());
    }
}

void Birdview::boundingbox()
{
    double temp, area, arclen = 0;
    int indexAreaMax = 0;

    for (int i = 0; i < static_cast<int>(m_contours.size()); i++)
    {
        temp = cv::contourArea(m_contours[i]);
        if (temp > area)
        {
            arclen = cv::arcLength(m_contours[i], true);
            cv::approxPolyDP(m_contours[i], m_contourApprox, 0.02 * arclen, true);
            if (m_contourApprox.size() == 4)
            {
                area = temp;
                indexAreaMax = i;
            }
            else
            {
                continue;
            }
        }
    }
    cv::drawContours(m_imgInput, m_contours, indexAreaMax, cv::Scalar(0, 255, 0), 2, 8, m_hierarchy, 0, cv::Point());
    std::cout << "Contour Index:\t" << indexAreaMax << std::endl;
    std::cout << "Contour Edges:\t" << m_contourApprox.size() << std::endl;
}

void Birdview::viewpoints()
{
    std::vector<int> pointOfView;
    int idxPoint;

    std::cout << "Contour Edge 0:\t" << m_contourApprox[0].x << " " << m_contourApprox[0].y << std::endl;
    std::cout << "Contour Edge 1:\t" << m_contourApprox[1].x << " " << m_contourApprox[1].y << std::endl;
    std::cout << "Contour Edge 2:\t" << m_contourApprox[2].x << " " << m_contourApprox[2].y << std::endl;
    std::cout << "Contour Edge 3:\t" << m_contourApprox[3].x << " " << m_contourApprox[3].y << std::endl;

    m_contourFinal.resize(4);
    for (int i = 0; i < static_cast<int>(m_contourApprox.size()); i++)
    {
        pointOfView.push_back(m_contourApprox[i].x + m_contourApprox[i].y);
    }
    idxPoint = std::min_element(std::begin(pointOfView), std::end(pointOfView)) - pointOfView.begin();
    m_contourFinal[0] = m_contourApprox[idxPoint];
    idxPoint = std::max_element(std::begin(pointOfView), std::end(pointOfView)) - pointOfView.begin();
    m_contourFinal[2] = m_contourApprox[idxPoint];

    pointOfView.clear();
    for (int i = 0; i < static_cast<int>(m_contourApprox.size()); i++)
    {
        pointOfView.push_back(m_contourApprox[i].x - m_contourApprox[i].y);
    }
    idxPoint = std::max_element(std::begin(pointOfView), std::end(pointOfView)) - pointOfView.begin();
    m_contourFinal[1] = m_contourApprox[idxPoint];
    idxPoint = std::min_element(std::begin(pointOfView), std::end(pointOfView)) - pointOfView.begin();
    m_contourFinal[3] = m_contourApprox[idxPoint];

    cv::putText(m_imgInput, "TL", m_contourFinal[0], 5, 1, cv::Scalar(0, 0, 255), 2);
    cv::putText(m_imgInput, "TR", m_contourFinal[1], 5, 1, cv::Scalar(0, 0, 255), 2);
    cv::putText(m_imgInput, "BR", m_contourFinal[2], 5, 1, cv::Scalar(0, 0, 255), 2);
    cv::putText(m_imgInput, "BL", m_contourFinal[3], 5, 1, cv::Scalar(0, 0, 255), 2);
}

void Birdview::transform()
{
    int aWidth, bWidth, maxWidth;
    int aHeight, bHeight, maxHeight;
    double distance;

    distance = std::sqrt(std::pow(m_contourFinal[2].x - m_contourFinal[3].x, 2) + std::pow(m_contourFinal[2].y - m_contourFinal[3].y, 2));
    aWidth = static_cast<int>(distance);
    distance = std::sqrt(std::pow(m_contourFinal[1].x - m_contourFinal[0].x, 2) + std::pow(m_contourFinal[1].y - m_contourFinal[0].y, 2));
    bWidth = static_cast<int>(distance);
    maxWidth = (aWidth > bWidth) ? aWidth : bWidth;

    distance = std::sqrt(std::pow(m_contourFinal[1].x - m_contourFinal[2].x, 2) + std::pow(m_contourFinal[1].y - m_contourFinal[2].y, 2));
    aHeight = static_cast<int>(distance);
    distance = std::sqrt(std::pow(m_contourFinal[0].x - m_contourFinal[3].x, 2) + std::pow(m_contourFinal[0].y - m_contourFinal[3].y, 2));
    bHeight = static_cast<int>(distance);
    maxHeight = (aHeight > bHeight) ? aHeight : bHeight;

    cv::Point2f sourcePoints[4], destPoints[4];
    cv::Mat transMatrix;

    sourcePoints[0] = cv::Point2f(m_contourFinal[0].x, m_contourFinal[0].y);
    sourcePoints[1] = cv::Point2f(m_contourFinal[1].x, m_contourFinal[1].y);
    sourcePoints[2] = cv::Point2f(m_contourFinal[2].x, m_contourFinal[2].y);
    sourcePoints[3] = cv::Point2f(m_contourFinal[3].x, m_contourFinal[3].y);

    destPoints[0] = cv::Point2f(0, 0);
    destPoints[1] = cv::Point2f(maxWidth - 1, 0);
    destPoints[2] = cv::Point2f(maxWidth - 1, maxHeight - 1);
    destPoints[3] = cv::Point2f(0, maxHeight - 1);

    transMatrix = cv::getPerspectiveTransform(sourcePoints, destPoints);
    cv::warpPerspective(m_imgInputClone, m_imgTransform, transMatrix, cv::Size(maxWidth, maxHeight));
}

void Birdview::debug(const modes &level)
{
    if (!m_imgInput.empty())
    {
        switch (level)
        {
            case INPUT: cv::imshow("m_imgInput", m_imgInput); break;
            case SMOOTH: cv::imshow("m_imgSmooth", m_imgSmooth); break;
            case CANNY: cv::imshow("m_imgCanny", m_imgCanny); break;
            case CONTOURS: cv::imshow("m_imgContours", m_imgContours); break;
            case TRANSFORM: cv::imshow("m_imgTransform", m_imgTransform); break;
            default: cv::imshow("m_imgInput", m_imgInput); break;
        }
        cv::waitKey(0);
    }
    else
    {
        std::cout << "No image initialized!" << std::endl;
    }
}