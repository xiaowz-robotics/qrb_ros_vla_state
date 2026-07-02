/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
 
#ifndef MODEL_HPP_
#define MODEL_HPP_

#include <sys/time.h>

#include <cstring>
#include <iostream>
#include <set>
#include <fstream>
#include <memory>
#include <vector>
#include <chrono>
#include <dlfcn.h>

#include "io_tensor.hpp"

#include <QnnInterface.h>
#include "System/QnnSystemInterface.h"

#include "HTP/QnnHtpPerfInfrastructure.h"
#include "HTP/QnnHtpDevice.h"
namespace qrb
{
namespace vla
{

template <typename Fn>
Fn loadQnnFunction(void *handle, const char *function_name)
{
    return reinterpret_cast<Fn>(dlsym(handle, function_name));
}

using QnnInterfaceGetProvidersFn = decltype(QnnInterface_getProviders);
using QnnSystemInterfaceGetProvidersFn = decltype(QnnSystemInterface_getProviders);


class Model
{
public:
    Model();
    bool Init_model(const std::string &model_path);
    ~Model();
    void Inference(std::vector<float> input_data, std::vector<std::vector<float>> &output_tensor, std::string perfProfile);
    void Inference_text(std::vector<int32_t> input_data, std::vector<std::vector<float>> &output_tensor, std::string perfProfile);
    bool copyMetadataToGraphsInfo(const QnnSystemContext_BinaryInfo_t *binaryInfo,
                                  GraphInfo_t **&graphsInfo,
                                  uint32_t &graphsCount);
    bool copyGraphsInfo(const QnnSystemContext_GraphInfo_t *graphsInput,
                        const uint32_t numGraphs,
                        GraphInfo_t **&graphsInfo);
    bool boostPerformance(QnnHtpDevice_PerfInfrastructure_t perfInfra, std::string perfProfile);
    bool resetPerformance(QnnHtpDevice_PerfInfrastructure_t perfInfra);
    bool disableDcvs(QnnHtpDevice_PerfInfrastructure_t perfInfra);
    bool enableDcvs(QnnHtpDevice_PerfInfrastructure_t perfInfra);
    bool copyGraphsInfoV3(const QnnSystemContext_GraphInfoV3_t *graphInfoSrc, GraphInfo_t *graphInfoDst);
    bool copyGraphsInfoV1(const QnnSystemContext_GraphInfoV1_t *graphInfoSrc, GraphInfo_t *graphInfoDst);
    bool copyTensorsInfo(const Qnn_Tensor_t *tensorsInfoSrc, Qnn_Tensor_t *&tensorWrappers, uint32_t tensorsCount);
    void FreeContext();
    void FreeDevice();
    void FreeBackend();
    QNN_SYSTEM_INTERFACE_VER_TYPE QNN_Sys_Interface;
    QNN_INTERFACE_VER_TYPE QNN_Interface;
    
    Qnn_DeviceHandle_t Device_handle;
    void *Backend_handle = nullptr;
    void *Sys_handle = nullptr;
    Qnn_ContextHandle_t Context = nullptr;
    bool foundValidInterface = false;
    bool SupportDevice = false;

private:

    IOTensor Q_IOTensor;
    uint32_t Q_GraphsCount = 0;
    uint32_t Num_Providers = 0;
    uint32_t Q_PowerConfigId = 1;
    QnnHtpDevice_PerfInfrastructure_t Q_PerfInfra = {nullptr};
    bool Q_RunInCpu = false;
    GraphInfo_t **Q_GraphsInfo;
    const std::string QNN_SYSLIB_PATH = "/usr/lib/libQnnSystem.so";
    const std::string LIB_PATH = "/usr/lib/libQnnHtp.so";
};
}  // namespace vla
}  // namespace qrb
#endif