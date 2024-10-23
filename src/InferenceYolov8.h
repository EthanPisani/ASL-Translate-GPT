#ifndef INFERENCEYOLOV8_H
#define INFERENCEYOLOV8_H

#include <opencv2/opencv.hpp>

#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <assert.h>
#include <chrono>
#include <onnxruntime/core/session/onnxruntime_cxx_api.h>

#include <onnxruntime/core/session/experimental_onnxruntime_cxx_api.h>

// class detection
class OnnxYoloInfer {
public:
    struct Detection {
        cv::Rect box;
        float conf{};
        int classId{};
    };

    template <typename T>
    T clip(const T& n, const T& lower, const T& upper);

    cv::Rect2f scaleCoords(const cv::Size& imageShape, cv::Rect2f coords, const cv::Size& imageOriginalShape, bool p_Clip = false);

    void getBestClassInfo(const cv::Mat& p_Mat, const int& numClasses,
                                    float& bestConf, int& bestClassId);
    std::vector<Detection> postprocessing(const cv::Size& resizedImageShape,
                                                    const cv::Size& originalImageShape,
                                                    std::vector<Ort::Value>& outputTensors,
                                                    const float& confThreshold, const float& iouThreshold);
    void letterbox(const cv::Mat& image, cv::Mat& outImage,
                      const cv::Size& newShape = cv::Size(640, 640),
                      const cv::Scalar& color = cv::Scalar(114, 114, 114),
                      bool auto_ = true,
                      bool scaleFill = false,
                      bool scaleUp = true,
                      int stride = 32);
    int calculate_product(const std::vector<int64_t>& v);

    std::string print_shape(const std::vector<int64_t>& v);

    Ort::Experimental::Session get_session(std::string &model_file, Ort::Env &env, bool useGPU);

    std::vector<Detection> detect(const cv::Mat &image, Ort::Experimental::Session &session);

    cv::Mat annotate_image(const cv::Mat &input_image, std::vector<Detection> &result);

    std::vector<std::string> simple_detect(cv::Mat &image);
};

#endif  // INFERENCEYOLOV8_H
