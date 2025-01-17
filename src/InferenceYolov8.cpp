#include "InferenceYolov8.h"
using namespace cv;

template <typename T>
T OnnxYoloInfer::clip(const T& n, const T& lower, const T& upper)
{
    return std::max(lower, std::min(n, upper));
}

cv::Rect2f OnnxYoloInfer::scaleCoords(const cv::Size& imageShape, cv::Rect2f coords, const cv::Size& imageOriginalShape, bool p_Clip)
{
    cv::Rect2f l_Result;
    float gain = std::min((float)imageShape.height / (float)imageOriginalShape.height,
                          (float)imageShape.width / (float)imageOriginalShape.width);

    int pad[2] = { (int)std::round((( (float)imageShape.width - (float)imageOriginalShape.width * gain) / 2.0f)-0.1f),
                 (int)std::round((( (float)imageShape.height - (float)imageOriginalShape.height * gain) / 2.0f)-0.1f)};

    l_Result.x = (int) std::round(((float)(coords.x - pad[0]) / gain));
    l_Result.y = (int) std::round(((float)(coords.y - pad[1]) / gain));

    l_Result.width = (int) std::round(((float)coords.width / gain));
    l_Result.height = (int) std::round(((float)coords.height / gain));

    // // clip coords, should be modified for width and height
    if (p_Clip)
    {
        l_Result.x = OnnxYoloInfer::clip(l_Result.x, (float)0, (float)imageOriginalShape.width);
        l_Result.y = OnnxYoloInfer::clip(l_Result.y, (float)0, (float)imageOriginalShape.height);
        l_Result.width = OnnxYoloInfer::clip(l_Result.width, (float)0, (float)(imageOriginalShape.width-l_Result.x));
        l_Result.height = OnnxYoloInfer::clip(l_Result.height, (float)0, (float)(imageOriginalShape.height-l_Result.y));
    }
    return l_Result;
}

void OnnxYoloInfer::getBestClassInfo(const cv::Mat& p_Mat, const int& numClasses,
                                    float& bestConf, int& bestClassId)
{
    bestClassId = 0;
    bestConf = 0;

    if (p_Mat.rows && p_Mat.cols)
    {
        for (int i = 0; i < numClasses; i++)
        {
            if (p_Mat.at<float>(0, i+4) > bestConf)
            {
                bestConf = p_Mat.at<float>(0, i+4);
                bestClassId = i;
            }
        }
    }
}

std::vector<OnnxYoloInfer::Detection> OnnxYoloInfer::postprocessing(const cv::Size& resizedImageShape,
                                                    const cv::Size& originalImageShape,
                                                    std::vector<Ort::Value>& outputTensors,
                                                    const float& confThreshold, const float& iouThreshold)
{
    std::vector<cv::Rect> boxes;
    std::vector<cv::Rect> nms_boxes;
    std::vector<float> confs;
    std::vector<int> classIds;

    auto* rawOutput = outputTensors[0].GetTensorData<float>();
    std::vector<int64_t> outputShape = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
    size_t count = outputTensors[0].GetTensorTypeAndShapeInfo().GetElementCount();

    cv::Mat l_Mat = cv::Mat(outputShape[1], outputShape[2], CV_32FC1, (void*)rawOutput);
    cv::Mat l_Mat_t = l_Mat.t();
    //std::vector<float> output(rawOutput, rawOutput + count);

    // for (const int64_t& shape : outputShape)
    //     std::cout << "Output Shape: " << shape << std::endl;

    // first 5 elements are box[4] and obj confidence
    int numClasses = l_Mat_t.cols - 4;

    // only for batch size = 1

    for (int l_Row = 0; l_Row < l_Mat_t.rows; l_Row++)
    {
        cv::Mat l_MatRow = l_Mat_t.row(l_Row);
        float objConf;
        int classId;

        OnnxYoloInfer::getBestClassInfo(l_MatRow, numClasses, objConf, classId);

        if (objConf > confThreshold)
        {
            float centerX = (l_MatRow.at<float>(0, 0));
            float centerY = (l_MatRow.at<float>(0, 1));
            float width = (l_MatRow.at<float>(0, 2));
            float height = (l_MatRow.at<float>(0, 3));
            float left = centerX - width / 2;
            float top = centerY - height / 2;

            float confidence = objConf;
            cv::Rect2f l_Scaled = OnnxYoloInfer::scaleCoords(resizedImageShape, cv::Rect2f(left, top, width, height), originalImageShape, true);

            // Prepare NMS filtered per class id's
            nms_boxes.emplace_back((int)std::round(l_Scaled.x)+classId*7680, (int)std::round(l_Scaled.y)+classId*7680, 
                (int)std::round(l_Scaled.width), (int)std::round(l_Scaled.height));
            boxes.emplace_back((int)std::round(l_Scaled.x), (int)std::round(l_Scaled.y), 
                (int)std::round(l_Scaled.width), (int)std::round(l_Scaled.height));
            confs.emplace_back(confidence);
            classIds.emplace_back(classId);
        }
    }

    std::vector<int> indices;
    cv::dnn::NMSBoxes(nms_boxes, confs, confThreshold, iouThreshold, indices);
    // std::cout << "amount of NMS indices: " << indices.size() << std::endl;

    std::vector<OnnxYoloInfer::Detection> detections;

    for (int idx : indices)
    {
        OnnxYoloInfer::Detection det;
        det.box = boxes[idx];
        det.conf = confs[idx];
        det.classId = classIds[idx];
        detections.emplace_back(det);
    }

    return detections;
}


void OnnxYoloInfer::letterbox(const cv::Mat& image, cv::Mat& outImage,
                      const cv::Size& newShape,
                      const cv::Scalar& color,
                      bool auto_,
                      bool scaleFill,
                      bool scaleUp,
                      int stride)
{
    cv::Size shape = image.size();
    float r = std::min((float)newShape.height / (float)shape.height,
                       (float)newShape.width / (float)shape.width);
    if (!scaleUp)
        r = std::min(r, 1.0f);

    float ratio[2] {r, r};
    int newUnpad[2] {(int)std::round((float)shape.width * r),
                     (int)std::round((float)shape.height * r)};

    auto dw = (float)(newShape.width - newUnpad[0]);
    auto dh = (float)(newShape.height - newUnpad[1]);

    if (auto_)
    {
        dw = (float)((int)dw % stride);
        dh = (float)((int)dh % stride);
    }
    else if (scaleFill)
    {
        dw = 0.0f;
        dh = 0.0f;
        newUnpad[0] = newShape.width;
        newUnpad[1] = newShape.height;
        ratio[0] = (float)newShape.width / (float)shape.width;
        ratio[1] = (float)newShape.height / (float)shape.height;
    }

    dw /= 2.0f;
    dh /= 2.0f;

    if (shape.width != newUnpad[0] || shape.height != newUnpad[1])
    {
        cv::resize(image, outImage, cv::Size(newUnpad[0], newUnpad[1]));
    }

    int top = int(std::round(dh - 0.1f));
    int bottom = int(std::round(dh + 0.1f));
    int left = int(std::round(dw - 0.1f));
    int right = int(std::round(dw + 0.1f));
    cv::copyMakeBorder(outImage, outImage, top, bottom, left, right, cv::BORDER_CONSTANT, color);
}


int OnnxYoloInfer::calculate_product(const std::vector<int64_t> &v)
{
    int total = 1;
    for (auto &i : v)
        total *= i;
    return total;
}

// pretty prints a shape dimension vector
std::string OnnxYoloInfer::print_shape(const std::vector<int64_t> &v)
{
    std::stringstream ss("");
    for (size_t i = 0; i < v.size() - 1; i++)
        ss << v[i] << "x";
    ss << v[v.size() - 1];
    return ss.str();
}


Ort::Experimental::Session OnnxYoloInfer::get_session(std::string &model_file, Ort::Env &env, bool useGPU)
{
    Ort::SessionOptions session_options;
    if (useGPU)
    {
        OrtCUDAProviderOptions l_CudaOptions;
        l_CudaOptions.device_id = 0;
        std::cout << "Before setting session options" << std::endl;
        session_options.AppendExecutionProvider_CUDA(l_CudaOptions);
        std::cout << "set session options" << std::endl;
    }
    else
    {
        // session_options.SetIntraOpNumThreads(12);
    }

    return Ort::Experimental::Session(env, model_file, session_options);
}




// main detect function (same as the one in main)
std::vector<OnnxYoloInfer::Detection> OnnxYoloInfer::detect(const cv::Mat &image, Ort::Experimental::Session &session)
{
    // print name/shape of inputs
    // std::vector<std::string> input_names = session.GetInputNames();
    std::vector<std::vector<int64_t>> input_shapes = session.GetInputShapes();
    // std::cout << "Input Node Name/Shape (" << input_names.size() << "):" << std::endl;
    // for (size_t i = 0; i < input_names.size(); i++)
    // {
    //     std::cout << "\t" << input_names[i] << " : " << print_shape(input_shapes[i]) << std::endl;
    // }

    // // print name/shape of outputs
    // std::vector<std::string> output_names = session.GetOutputNames();
    std::vector<std::vector<int64_t>> output_shapes = session.GetOutputShapes();
    // std::cout << "Output Node Name/Shape (" << output_names.size() << "):" << std::endl;
    // for (size_t i = 0; i < output_names.size(); i++)
    // {
    //     std::cout << "\t" << output_names[i] << " : " << print_shape(output_shapes[i]) << std::endl;
    // }


    

    cv::Mat resizedImage, floatImage;
    cv::cvtColor(image, resizedImage, cv::COLOR_BGR2RGB);
    OnnxYoloInfer::letterbox(resizedImage, resizedImage, cv::Size(640, 640),
                    cv::Scalar(114, 114, 114), false,
                    false, true, 32);
    resizedImage.convertTo(floatImage, CV_32FC3, 1 / 255.0);
    float* blob = new float[floatImage.cols * floatImage.rows * floatImage.channels()];
    cv::Size floatImageSize {floatImage.cols, floatImage.rows};
    std::vector<cv::Mat> chw(floatImage.channels());
    for (int i = 0; i < floatImage.channels(); ++i)
    {
        chw[i] = cv::Mat(floatImageSize, CV_32FC1, blob + i * floatImageSize.width * floatImageSize.height);
    }
    cv::split(floatImage, chw);
    std::vector<float> inputTensorValues(blob, blob+3*floatImageSize.width*floatImageSize.height);
    std::vector<Ort::Value> input_tensors;
    input_tensors.push_back(Ort::Experimental::Value::CreateTensor<float>(inputTensorValues.data(), inputTensorValues.size(), input_shapes[0]));
    auto output_tensors = session.Run(session.GetInputNames(), input_tensors, session.GetOutputNames());
    std::vector<OnnxYoloInfer::Detection> result = postprocessing(cv::Size(640, 640), image.size(), output_tensors, 0.5, 0.45);

    return result;
}

cv::Mat OnnxYoloInfer::annotate_image(const cv::Mat &input_image, std::vector<OnnxYoloInfer::Detection> &result)
{
    cv::Mat output_image = input_image.clone();
    for (auto detection : result)
    {
        cv::Point l_P1(detection.box.x, detection.box.y);
        cv::Point l_P2(detection.box.x+detection.box.width, detection.box.y+detection.box.height);
        cv::rectangle(output_image, l_P1, l_P2, cv::Scalar(0, 255, 0), 1);
        std::stringstream l_ss;
        l_ss << "Object ID:" << int(detection.classId) << " Score:";
        // class_image_id << "obj" << int(detection.classId) << "_score" << std::round(detection.conf*100)/100.0;
        l_ss.setf(std::ios::fixed);
        l_ss.precision(2);
        l_ss << detection.conf;
        std::string l_Label = l_ss.str();
        std::cout << l_Label << std::endl;
        int l_BaseLine = 0;
        cv::Size l_TextSize = cv::getTextSize(l_Label, 0, 0.5, 1, &l_BaseLine);
        bool l_outSide = false;
        if (l_P1.y - l_TextSize.height >= 3)
        {
            l_outSide = true;
        }
        cv::Point l_P3;
        l_P3.x = l_P1.x+l_TextSize.width;
        if (l_outSide)
        {
            l_P3.y = l_P1.y-l_TextSize.height-3;
        }
        else
        {
            l_P3.y = l_P1.y+l_TextSize.height+3;
        }
        cv::rectangle(output_image, l_P1, l_P3, cv::Scalar(0, 255, 0), -1);
        cv::Point l_TextPos;
        l_TextPos.x = l_P1.x;
        if (l_outSide)
        {
            l_TextPos.y = l_P1.y - 2;
        }
        else
        {
            l_TextPos.y = l_P1.y + l_TextSize.height + 2;
        }
        cv::putText(output_image, l_Label, l_TextPos, 0, 0.5, cv::Scalar(0, 0, 0));
    }

    return output_image;
}
// return the character of the detected sign given only a cv::Mat
std::vector<std::string> OnnxYoloInfer::simple_detect(cv::Mat &image) {
    // use cpu and create session
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "example-model-explorer");
    std::string model_file = "./models/best640.onnx";
    Ort::Experimental::Session session = OnnxYoloInfer::get_session(model_file, env, false);

    // detect
    std::vector<OnnxYoloInfer::Detection> result = detect(image, session);

    // convert the top detection to a string (map object ID to character)
    std::vector<std::string> output;
    for (auto detection : result)
    {
        int classId = detection.classId;
        // convert int to char where 0 is a and 25 is z
        std::string l_Label = std::string(1, (char)(classId+97));
        output.push_back(l_Label);
    }
    return output;
}


// int main(int argc,char *argv[]){

//     // make a new instance of the class
//     OnnxYoloInfer infer;
//     // do a simple detect
//     cv::Mat image = cv::imread(argv[1], 1);

//     std::vector<std::string> output = infer.simple_detect(image);
//     // print the output
//     for (auto detection : output)
//     {
//         std::cout << detection << std::endl;
//     }
// }
