#include <fmt/core.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <numeric>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ximgproc.hpp>
#include <stdexcept>

#include "utils.hpp"

using cv::Mat;
using cv::Ptr;
using cv::Rect;

using cv::ximgproc::EdgeBoxes;
using cv::ximgproc::StructuredEdgeDetection;

using cv::ximgproc::createEdgeBoxes;
using cv::ximgproc::createStructuredEdgeDetection;

#define model_file "data/model.yml"

auto load_edgedetect_model() -> Ptr<StructuredEdgeDetection> {
    auto edge_detect = timeit("edge detector model load", []() {
       return createStructuredEdgeDetection(model_file);
   }).trace();

    return edge_detect;
}

auto load_edgebox_detect() -> Ptr<EdgeBoxes> {
    auto edgebox_detector = timeit("edgebox detector load", []() {
        auto edgeboxes = createEdgeBoxes();
        edgeboxes->setMaxBoxes(10);

        return edgeboxes;
    }).trace();

    return edgebox_detector;
}

auto load_image(std::string image_file) -> Mat {
    Mat im = cv::imread(image_file);

    Mat rgb_im;
    cv::cvtColor(im, rgb_im, cv::COLOR_BGR2RGB);
    rgb_im.convertTo(rgb_im, CV_32F, 1.0 / 255.0f);

    return rgb_im;
}

auto detect_edges(Ptr<StructuredEdgeDetection> edge_detector, Mat im) -> Mat {
    Mat edge_im;
    timeit("edge detect", [&]() {
        edge_detector->detectEdges(im, edge_im);
    }).trace();

    return edge_im;
}

auto compute_orientation(
    Ptr<StructuredEdgeDetection> edge_detector, Mat edge_im) -> Mat {
    Mat O;
    timeit("compute orientation", [&]() {
        edge_detector->computeOrientation(edge_im, O);
    }).trace();

    return O;
}

auto do_nms(Ptr<StructuredEdgeDetection> edge_detector, Mat edge_im, Mat O)
    -> Mat {
    Mat edge_nms;

    timeit("nms", [&]() {
        auto r = 2;
        auto s = 0;
        auto m = 1;
        auto is_parallel = true;
        edge_detector->edgesNms(edge_im, O, edge_nms, r, s, m, is_parallel);
    }).trace();

    return edge_nms;
}

auto find_boxes(Ptr<EdgeBoxes> box_detector, Mat edge_nms, Mat O)
    -> std::tuple<std::vector<Rect>, std::vector<float>> {
    return timeit("find boxes", [&]() {
        std::vector<float> scores;
        std::vector<Rect> boxes;

        box_detector->getBoundingBoxes(edge_nms, O, boxes, scores);

        return std::make_tuple(boxes, scores);
    }).trace();
}

auto write_boxes(Mat im, std::vector<Rect> boxes, std::vector<float> scores) {
    const auto green = cv::Scalar(0, 255, 0);
    const auto red = cv::Scalar(0, 0, 255);

    auto [min_score, max_score] = minmax_element(scores.begin(), scores.end());
    auto avg_score = std::reduce(scores.begin(), scores.end()) / scores.size();
    auto score75 = *max_score - (avg_score / 2);
    fmt::print(
        "stats: min={}, average={}, max={}.\n",
        *min_score,
        avg_score,
        *max_score);

    for (size_t i = 0; i < boxes.size(); i++) {
        auto box = boxes[i];
        // left-top corner
        cv::Point p1 { box.x, box.y };
        // right-bottom corner
        cv::Point p2 { box.x + box.width, box.y + box.height };

        auto [color, thickness] = scores[i] < score75
            ? std::make_tuple(red, 1)
            : std::make_tuple(green, 3);
        cv::rectangle(im, p1, p2, color, thickness);
    }
}

auto show_image(std::string name, Mat im) {
    cv::imshow(name, im);
    cv::moveWindow(name, 0, 0);

    int key = 0;
    while (key != 'q') {
        key = cv::waitKey(0);
    }
    cv::destroyAllWindows();
}

auto main(int argc, char** argv) -> int {
    if (argc < 2) {
        throw std::runtime_error("please provide image filename");
    }

    auto image_filename = argv[1];

    auto edge_detector = load_edgedetect_model();
    auto edgebox_detector = load_edgebox_detect();

    auto im = load_image(image_filename);
    fmt::print("\nimage {}x{}\n", im.rows, im.cols);

    auto edge_im = detect_edges(edge_detector, im);
    auto O = compute_orientation(edge_detector, edge_im);
    auto edge_nms = do_nms(edge_detector, edge_im, O);

    auto [boxes, scores] = find_boxes(edgebox_detector, edge_nms, O);

    Mat display;
    cv::cvtColor(edge_im, display, cv::COLOR_GRAY2RGB);
    write_boxes(display, boxes, scores);
    show_image("image with boxes", display);

    return 0;
}
