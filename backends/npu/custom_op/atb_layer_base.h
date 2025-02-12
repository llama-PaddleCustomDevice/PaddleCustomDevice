// Copyright (c) 2023 PaddlePaddle Authors. All Rights Reserved.
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

#pragma once
#ifdef PADDLE_WITH_ASCEND_TRANSFORMER_ACC
#include <acl/acl.h>
#include "atb/atb_infer.h"
#include "paddle/extension.h"
#include "atb/context.h"

class PpAscendAtbOpBase {
public:
  PpAscendAtbOpBase(const std::string &opName);
  ~PpAscendAtbOpBase();

  virtual void BuildVariantPack(std::vector<const phi::DenseTensor *> &inTensors,
                                std::vector<const phi::DenseTensor *> &outTensors);
  virtual atb::Status Execute(aclrtStream stream,
                              std::vector<const phi::DenseTensor *> &inTensors,
                              std::vector<const phi::DenseTensor *> &outTensors);

  std::shared_ptr<atb::Operation> operation_;

protected:
  std::string opName_;
  atb::VariantPack variantPacks_;
  aclrtStream stream_;
  atb::Context *context_ = nullptr;
  void *workspace_ = nullptr;
  void SetWorkspace(uint64_t workspace_size);

private:
  uint64_t workspaceSize_ = 0;
  int32_t currentDevId_ = 0;
};

#include <atomic>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>


class PpAscendAtbOpBaseAsync {
public:
  PpAscendAtbOpBaseAsync(const std::string &opName);
  ~PpAscendAtbOpBaseAsync();

  virtual void BuildVariantPack(std::vector<const phi::DenseTensor *> &inTensors,
                                std::vector<const phi::DenseTensor *> &outTensors,
                                uint64_t layerId);
                                
  void Setup(aclrtStream stream,
             std::vector<const phi::DenseTensor *> &inTensors,
             std::vector<const phi::DenseTensor *> &outTensors,
             uint64_t layerId);

  void Execute(aclrtStream stream, uint64_t layerId);
  void ThreadProcessTask();
  void PushTask(int layerId);
  int PopTask();
  void ExecutePlan(int layerId);

  std::shared_ptr<atb::Operation> operation_;

protected:
  std::string opName_;
  std::vector<atb::VariantPack> variantPacks_;
  std::thread taskProcessThread_;
  std::queue<int> taskQueue_;
  std::atomic_int layerCount_;
  std::mutex mutex_;
  std::condition_variable cond_;

private:
  void SetWorkspace(uint64_t workspace_size);

private:
  uint64_t workspaceSize_ = 0;
  void *workspace_ = nullptr;

   int32_t currentDevId_ = 0;
   int32_t layerNum_ = 0;

  aclrtStream stream_;

};
#endif
