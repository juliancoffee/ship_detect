# Introduction

Code based on OpenCV Structured Edge Detection implementation and Edge Boxes implementation.
Used to display object proposals.

# Build
We use CMake and git submodules for dependency handling (except OpenCV).

```bash
$ git submodule update --init --recursive
$ mkdir build
$ cd build
$ cmake ..
$ make
```

# Data
Example assumes that you have a folder `data` with `model.yml` in it. You can get it [here](https://github.com/opencv/opencv_extra/raw/master/testdata/cv/ximgproc/model.yml.gz).

# Run
Run build/example --help to see all available options

# Example
Run object detection on data/your_video.mp4 file and show 10 object proposals with highest criteria.
```bash
$ build/example --video data/your_video.mp4 --n 10
```
