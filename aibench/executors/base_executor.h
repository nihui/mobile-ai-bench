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

#ifndef AIBENCH_EXECUTORS_BASE_EXECUTOR_H_
#define AIBENCH_EXECUTORS_BASE_EXECUTOR_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace aibench {

enum Status {
  SUCCESS = 0,
  RUNTIME_ERROR = 1,
  NOT_SUPPORTED = 2
};

enum Framework {
  MACE = 0,
  SNPE = 1,
  NCNN = 2,
  TFLITE = 3
};

enum Runtime {
  CPU = 0,
  GPU = 1,
  DSP = 2
};

// input/output tensor
class BaseTensor {
 public:
  // shape - the shape of the tensor, with size n
  // data - the buffer of the tensor, must not be null with size equals
  //        shape[0] * shape[1] * ... * shape[n-1]
  explicit BaseTensor(const std::vector<int64_t> &shape,
                      std::shared_ptr<float> data);
  BaseTensor();
  BaseTensor(const BaseTensor &other);
  BaseTensor(const BaseTensor &&other);
  BaseTensor &operator=(const BaseTensor &other);
  BaseTensor &operator=(const BaseTensor &&other);
  ~BaseTensor();

  const std::vector<int64_t> &shape() const;
  const std::shared_ptr<float> data() const;
  int64_t size() const;
  std::shared_ptr<float> data();

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

class BaseExecutor {
 public:
  explicit BaseExecutor(Framework framework, Runtime runtime)
      : framework_(framework), runtime_(runtime) {}

  virtual ~BaseExecutor() = default;

  // If your framework needs to initialize something other than loading
  // model or creating an engine, you can put it here, e.g., Mace needs
  // to compile OpenCL kernel once per target.
  virtual Status Init(const char *model_name, int num_threads) = 0;

  // Load model and prepare to run.
  virtual Status Prepare(const char *model_name) = 0;

  // Run the model.
  virtual Status Run(const std::map<std::string, BaseTensor> &inputs,
                     std::map<std::string, BaseTensor> *outputs) = 0;

  // Unload model and free the memory after running the model.
  virtual void Finish() = 0;
  Framework GetFramework() {return framework_;}
  Runtime GetRuntime() {return runtime_;}

 private:
  Framework framework_;
  Runtime runtime_;
};

}  // namespace aibench

#endif  // AIBENCH_EXECUTORS_BASE_EXECUTOR_H_
