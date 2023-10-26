#include <fmt/core.h>
#include <fmt/ranges.h>

#include <argparse/argparse.hpp>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ximgproc.hpp>

#include <algorithm>
#include <numeric>
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

struct EdgeBoxOpts {
    int n;
    float edge_merge;
    float cluster_min_mag;
    float min_box_area;
};

auto load_edgebox_detect(EdgeBoxOpts opts) -> Ptr<EdgeBoxes> {
    auto edgebox_detector = timeit("edgebox detector load", [=]() {
        auto edgeboxes = createEdgeBoxes();
        edgeboxes->setMaxBoxes(opts.n);
        edgeboxes->setEdgeMergeThr(opts.edge_merge);
        edgeboxes->setClusterMinMag(opts.cluster_min_mag);
        edgeboxes->setMinBoxArea(opts.min_box_area);

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
    const auto blue = cv::Scalar(255, 0, 0);
    const auto red = cv::Scalar(0, 0, 255);

    auto [min_score, max_score] = minmax_element(scores.begin(), scores.end());
    auto avg_score = std::reduce(scores.begin(), scores.end()) / scores.size();
    auto score75 = *max_score - (avg_score / 2.0);
    auto score95 = *max_score - (avg_score / 10.0);
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

        if (scores[i] > score95) {
            cv::rectangle(im, p1, p2, green, 3);
        } else if (scores[i] > score75) {
            cv::rectangle(im, p1, p2, blue, 2);
        } else {
            cv::rectangle(im, p1, p2, red, 1);
        }
    }
}

auto draw_contours(Mat src) -> Mat {
    using namespace cv;

    // Convert to CV_8UC1 if not already
    if (src.type() != CV_8UC1) {
        src.convertTo(src, CV_8UC1, 255);  // Assuming the image is in range [0,1]
    }

    // Threshold the edge image to get a binary image
    threshold(src, src, 128, 255, THRESH_BINARY);
    Mat dst = Mat::zeros(src.rows, src.cols, CV_8UC3);

    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i> hierarchy;

    findContours(src, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);

    // iterate through all the top-level contours,
    // draw each connected component with its own random color
    for(int idx = 0; idx >= 0; idx = hierarchy[idx][0])
    {
        Scalar color(rand() & 255, rand() & 255, rand() & 255);
        drawContours(dst, contours, idx, color, FILLED, 8, hierarchy);
    }

    return dst;
}

auto show_image(std::string name, Mat im, int wait_for = 0, bool keep = false) {
    cv::imshow(name, im);
    cv::moveWindow(name, 0, 0);

    int key = 0;

    while (key != 'q') {
        key = cv::waitKey(wait_for);

        if (wait_for != 0) {
            break;
        }
    }

    auto destroy = !keep;
    if (destroy) {
        cv::destroyAllWindows();
    }
}

auto process_image(
        Ptr<StructuredEdgeDetection> edge_detector,
        Ptr<EdgeBoxes> edgebox_detector,
        Mat im,
        int wait_for = 0,
        bool keep = false)
{
    auto edge_im = detect_edges(edge_detector, im);
    auto O = compute_orientation(edge_detector, edge_im);
    auto edge_nms = do_nms(edge_detector, edge_im, O);

    auto [boxes, scores] = find_boxes(edgebox_detector, edge_nms, O);

    // Mat display;
    // cv::cvtColor(edge_im, display, cv::COLOR_GRAY2RGB);
    Mat display = draw_contours(edge_im);
    write_boxes(display, boxes, scores);
    show_image("image with boxes", display, wait_for, keep);
}

auto process_image(
        Ptr<StructuredEdgeDetection> edge_detector,
        Ptr<EdgeBoxes> edgebox_detector,
        std::string image_filename,
        int wait_for = 0,
        bool keep = false)
{
    auto im = load_image(image_filename);
    fmt::print("\nimage {}: {}x{}\n", image_filename, im.rows, im.cols);

    process_image(edge_detector, edgebox_detector, im, wait_for, keep);

}

auto process_video(
        Ptr<StructuredEdgeDetection> edge_detector,
        Ptr<EdgeBoxes> edgebox_detector,
        std::string video_filename)
{
    Mat frame_buffer;
    cv::VideoCapture capture;

    fmt::print("[capture object created]\n");
    capture.open(video_filename);

    if (!capture.isOpened()) {
        fmt::print("[video capture error]\n");
        return; // Exit the function if the video cannot be opened.
    }
    fmt::print("[video captured]\n");

    while (true) {
        auto res = capture.read(frame_buffer);
        if (!res) {
            fmt::print("[error reading frame]\n");
            break; // Exit the loop if there's an error reading a frame.
        }
        fmt::print("[got frame read]\n");

        Mat input_im;
        frame_buffer.convertTo(input_im, CV_32FC3, 1.0 / 255.0);

        fmt::print("[image converted]\n");

        process_image(edge_detector, edgebox_detector, input_im, 1, true);
    }
}

struct MyArgs : argparse::Args {
    std::string &filename = arg("file name");
    bool &video = flag("video", "video mode");
    int &n = kwarg("n", "number of boxes").set_default(5);
    float &min_box_area = kwarg("area", "min box").set_default(1000.0);
    float &edge_merge = kwarg("merge", "edge merge threshold").set_default(0.5);
    float &cluster_min_mag = kwarg("cluster", "cluster min magnitude").set_default(0.5);
};

auto main(int argc, char** argv) -> int {
    MyArgs args = argparse::parse<MyArgs>(argc, argv);


    auto edge_detector = load_edgedetect_model();

    int n = args.n;
    float min_box_area = args.min_box_area;
    float edge_merge = args.edge_merge;
    float cluster_min_mag = args.cluster_min_mag;
    EdgeBoxOpts opts {n, edge_merge, cluster_min_mag, min_box_area};
    auto edgebox_detector = load_edgebox_detect(opts);

    if (args.video) {
        auto video_filename = args.filename;
        process_video(edge_detector, edgebox_detector, video_filename);
    } else {
        auto image_filename = args.filename;
        process_image(edge_detector, edgebox_detector, image_filename);
    }

    return 0;
}
