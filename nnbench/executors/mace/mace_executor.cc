// Copyright 2018 Xiaomi, Inc.  All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "nnbench/executors/mace/mace_executor.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

namespace nnbench {

inline Status ReadBinaryFile(std::vector<unsigned char> *data,
                           const std::string &filename) {
  std::ifstream ifs(filename, std::ios::in | std::ios::binary);
  if (!ifs.is_open()) {
    return Status::RUNTIME_ERROR;
  }
  ifs.seekg(0, ifs.end);
  size_t length = ifs.tellg();
  ifs.seekg(0, ifs.beg);

  data->reserve(length);
  data->insert(data->begin(), std::istreambuf_iterator<char>(ifs),
               std::istreambuf_iterator<char>());
  if (ifs.fail()) {
    return Status::RUNTIME_ERROR;
  }
  ifs.close();

  return Status::SUCCESS;
}

Status MaceCPUExecutor::Prepare(const char *model_name) {
  mace::DeviceType device_type = mace::CPU;
  mace::SetOpenMPThreadPolicy(4, static_cast<mace::CPUAffinityPolicy>(0));

  std::vector<unsigned char> model_pb_data;
  std::string model_pb_file(model_name);
  model_pb_file.append(".pb");
  if (ReadBinaryFile(&model_pb_data, model_pb_file) != Status::SUCCESS) {
    std::cout << "Failed to read file: " << model_name << std::endl;
    return Status::RUNTIME_ERROR;
  }
  std::string model_data_file(model_name);
  model_data_file.append(".data");

  mace::MaceStatus create_engine_status;
  create_engine_status =
      CreateMaceEngineFromProto(model_pb_data,
                                model_data_file,
                                input_names_,
                                output_names_,
                                device_type,
                                &engine_);

  return create_engine_status == mace::MACE_SUCCESS ? Status::SUCCESS
                                                    : Status::RUNTIME_ERROR;
}

Status MaceCPUExecutor::Run(const std::map<std::string, BaseTensor> &inputs,
                            std::map<std::string, BaseTensor> *outputs) {
  (void)outputs;

  std::map<std::string, mace::MaceTensor> mace_inputs;
  std::map<std::string, mace::MaceTensor> mace_outputs;
  for (auto input : inputs) {
    mace_inputs[input.first] = mace::MaceTensor(input.second.shape(),
                                                input.second.data());
  }

  mace::MaceStatus run_status = engine_->Run(mace_inputs, &mace_outputs);

  return run_status == mace::MACE_SUCCESS ? Status::SUCCESS
                                          : Status::RUNTIME_ERROR;
}

void MaceCPUExecutor::Finish() {
  engine_.reset();
}

}  // namespace nnbench