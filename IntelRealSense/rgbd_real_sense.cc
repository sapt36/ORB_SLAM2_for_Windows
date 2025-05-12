/**
 * This file is part of ORB-SLAM2.
 *
 * Copyright (C) 2014-2016 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
 * For more information see <https://github.com/raulmur/ORB-SLAM2>
 *
 * ORB-SLAM2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ORB-SLAM2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ORB-SLAM2. If not, see <http://www.gnu.org/licenses/>.
 *
 * RGB-D file 
 * Usage: 
 * ./build/rgbd_real_sense Vocabulary/ORBvoc.txt RealSense_GPU/rgbd_real_sense.yaml  
 *
 */

#include <iostream>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/opengl.hpp>
#include <System.h>
#include <Utils.hpp>
#include <Converter.h>
#include <SGFilter.h>
#include <librealsense2/rs.hpp>
#include <cv-helpers.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define POLY_ORDER 2
#define FILTER_SIZE 5

using namespace std;

float get_depth_scale(rs2::device dev);
rs2_stream find_stream_to_align(const vector<rs2::stream_profile>& streams);
bool profile_changed(const vector<rs2::stream_profile>& current, const vector<rs2::stream_profile>& prev);

bool operator!(const cv::Mat &m) { return m.empty(); }

volatile sig_atomic_t flag = 0;
void on_stop(int sig) { flag = 1; }

int main(int argc, char **argv)
{
    signal(SIGINT, on_stop);
    if(argc < 3)
    {
        cerr << endl << "Usage: ./csi_camera path_to_vocabulary path_to_settings" << endl;
        return 1;
    }
    else if(argc > 8)
    {
        cerr << endl << "Usage: ./csi_camera path_to_vocabulary path_to_settings" << endl;
        return 1;
    }

    int WIDTH, HEIGHT, FPS;
    double TIME;
    bool bUseViz = true;
    if(argc > 3)
    {
        stringstream ss(argv[3]);
        bool b;
        if(ss >> boolalpha >> b)
            bUseViz = b;
    }
    WIDTH = (argc > 4) ? atoi(argv[4]) : 640;
    HEIGHT = (argc > 5) ? atoi(argv[5]) : 480;
    FPS    = (argc > 6) ? atoi(argv[6]) : 30;
    TIME   = (argc > 7) ? atof(argv[7]) : 30000.0;

    // Construct a pipeline which abstracts the device
    rs2::pipeline pipe;

    // Create a configuration for configuring the pipeline with a non default profile
    rs2::config cfg;

    // Add desired streams to configuration
    cfg.enable_stream(RS2_STREAM_COLOR, WIDTH, HEIGHT, RS2_FORMAT_RGB8, FPS);
    cfg.enable_stream(RS2_STREAM_DEPTH, WIDTH, HEIGHT, RS2_FORMAT_Z16, FPS);

    // Start the camera pipeline
    rs2::pipeline_profile selection = pipe.start(cfg);

    // Get depth scale
    float depth_scale = get_depth_scale(selection.get_device());

    // Determine stream to align with
    rs2_stream align_to = find_stream_to_align(selection.get_streams());

    // Create align object
    rs2::align align(align_to);

    // Get camera intrinsics
    auto ds      = selection.get_stream(RS2_STREAM_COLOR).as<rs2::video_stream_profile>();
    auto intr    = ds.get_intrinsics();
    auto resolution      = make_pair(ds.width(), ds.height());
    auto principal_point = make_pair(intr.ppx, intr.ppy);
    auto focal_length    = make_pair(intr.fx, intr.fy);

    cout << "ppx: " << intr.ppx << " ppy: " << intr.ppy << endl;
    cout << "fx: " << intr.fx << " fy: " << intr.fy << endl;
    cout << "k1: " << intr.coeffs[0] << " k2: " << intr.coeffs[1]
         << " p1: " << intr.coeffs[2] << " p2: " << intr.coeffs[3]
         << " k3: " << intr.coeffs[4] << endl;

    // Camera warmup - drop first frames
    rs2::frameset frames;
    for(int i=0; i<30; i++)
    {
        frames = pipe.wait_for_frames();
        if(profile_changed(pipe.get_active_profile().get_streams(), selection.get_streams()))
        {
            selection = pipe.get_active_profile();
            align_to   = find_stream_to_align(selection.get_streams());
            align      = rs2::align(align_to);
            depth_scale= get_depth_scale(selection.get_device());
        }
        auto processed = align.process(frames);
    }

    // Initialize Savitzky-Golay Filters
    ORB_SLAM2::SGFilter xFilter(POLY_ORDER, FILTER_SIZE);
    ORB_SLAM2::SGFilter yFilter(POLY_ORDER, FILTER_SIZE);
    ORB_SLAM2::SGFilter zFilter(POLY_ORDER, FILTER_SIZE);

    // Create SLAM system
    ORB_SLAM2::System SLAM(argv[1], argv[2], ORB_SLAM2::System::RGBD, bUseViz);

    cout << endl << "-------" << endl;
    cout << "Start processing sequence ..." << endl;

    double tsum = 0, tbuf[10] = {0.0};
    int tpos = 0;
    double trackTimeSum = 0;
    int frameNumber = 0;

    // Main loop
    while(true)
    {
        frames = pipe.wait_for_frames();

        cv::Mat infared, depth;
        if(frameNumber % 150 == 0)
        {
            if(profile_changed(pipe.get_active_profile().get_streams(), selection.get_streams()))
            {
                selection = pipe.get_active_profile();
                align_to  = find_stream_to_align(selection.get_streams());
                align     = rs2::align(align_to);
                depth_scale = get_depth_scale(selection.get_device());
            }
            auto processed        = align.process(frames);
            rs2::video_frame of   = processed.first(align_to);
            rs2::depth_frame df   = processed.get_depth_frame();
            infared               = frame_to_mat(of);
            depth                 = frame_to_mat(df);
        }
        else
        {
            rs2::video_frame rf   = frames.first(RS2_STREAM_COLOR);
            rs2::depth_frame df   = frames.get_depth_frame();
            infared               = frame_to_mat(rf);
            depth                 = frame_to_mat(df);
        }

        // Compute timestamp and check end condition
        double tframe = chrono::duration_cast<chrono::duration<double>>(chrono::steady_clock::now() - chrono::steady_clock::time_point()).count();
        if(tframe > TIME) break;

        // Pass the image to the SLAM system
        cv::Mat Tcw = SLAM.TrackRGBD(infared, depth, tframe);

        ++frameNumber;

        if(flag)
        {
            cerr << "Mean track time: " << trackTimeSum/frameNumber
                 << " , mean fps: " << frameNumber / tframe << endl;
            SLAM.SaveKeyFrameTrajectoryTUM("KeyFrameTrajectory.txt");
            SLAM.Shutdown();
            return 0;
        }
    }

    SLAM.Shutdown();
    SLAM.SaveKeyFrameTrajectoryTUM("KeyFrameTrajectory.txt");
    return 0;
}

float get_depth_scale(rs2::device dev)
{
    for(auto &sensor : dev.query_sensors())
    {
        if(rs2::depth_sensor d = sensor.as<rs2::depth_sensor>())
            return d.get_depth_scale();
    }
    throw runtime_error("Device does not have a depth sensor");
}

rs2_stream find_stream_to_align(const vector<rs2::stream_profile>& streams)
{
    rs2_stream align_to = RS2_STREAM_ANY;
    bool depth_found = false, color_found = false;
    for(auto &sp : streams)
    {
        rs2_stream st = sp.stream_type();
        if(st != RS2_STREAM_DEPTH)
        {
            if(!color_found) align_to = st;
            if(st == RS2_STREAM_COLOR) color_found = true;
        }
        else depth_found = true;
    }
    if(!depth_found) throw runtime_error("No Depth stream available");
    if(align_to == RS2_STREAM_ANY) throw runtime_error("No stream found to align with Depth");
    return align_to;
}

bool profile_changed(const vector<rs2::stream_profile>& current, const vector<rs2::stream_profile>& prev)
{
    for(const auto &sp : prev)
    {
        auto itr = find_if(begin(current), end(current), [&sp](const rs2::stream_profile& cs){ return sp.unique_id() == cs.unique_id(); });
        if(itr == end(current))
            return true;
    }
    return false;
}
