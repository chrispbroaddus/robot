/*
   This file is part of the Calibu Project.
   https://github.com/gwu-robotics/Calibu

   Copyright (C) 2013 George Washington University,
                      Steven Lovegrove

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

#include <calibu/Platform.h>
#include <ceres/ceres.h>

namespace calibu {

class CostFunctionAndParams {
public:
    virtual ~CostFunctionAndParams() {}

    ceres::CostFunction*& Cost() { return m_cost; }

    std::vector<double*>& Params() { return m_params; }

    ceres::LossFunction*& Loss() { return m_loss_func; }

protected:
    ceres::CostFunction* m_cost;
    std::vector<double*> m_params;
    ceres::LossFunction* m_loss_func;
};
}
