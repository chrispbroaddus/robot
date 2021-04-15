/*
   This file is part of the Calibu Project.
   https://github.com/arpg/Calibu

   Copyright (C) 2013 George Washington University,
                      Steven Lovegrove,
                      Gabe Sibley

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 */

#pragma once

#include <sophus/se3.hpp>

#include <calibu/Platform.h>
#include <calibu/cam/camera_crtp.h>
#include <calibu/cam/camera_models_crtp.h>
#include <calibu/cam/rectify_crtp.h>

namespace calibu {

/// Create left and right camera lookup tables from left and right camera models,
/// and output their new intrinsics and extrinsics.
/// Returns: New camera rig (intrinsics same for both cameras)
/// T_nr_nl: New scanline rectified extrinsics considering rotation applied in lookup tables.
CALIBU_EXPORT
std::shared_ptr<calibu::Rig<double>> CreateScanlineRectifiedLookupAndCameras(const Sophus::SE3d& T_rl,
    const std::shared_ptr<calibu::CameraInterface<double>> cam_left, const std::shared_ptr<calibu::CameraInterface<double>> cam_right,
    Sophus::SE3d& T_nr_nl, LookupTable& left_lut, LookupTable& right_lut);
}
