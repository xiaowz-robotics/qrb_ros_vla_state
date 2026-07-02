/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#pragma once

#include <memory>
#include <queue>
#include <numeric>

#include "QnnBackend.h"
#include "QnnCommon.h"
#include "QnnContext.h"
#include "QnnGraph.h"
#include "QnnProperty.h"
#include "QnnTensor.h"
#include "QnnTypes.h"

#include "qnn_type_macros.hpp"
#include "data_util.hpp"
namespace qrb
{
namespace vla
{
enum class OutputDataType
{
    FLOAT_ONLY,
    NATIVE_ONLY,
    FLOAT_AND_NATIVE,
    INVALID
};
enum class InputDataType
{
    FLOAT,
    NATIVE,
    INVALID
};

typedef struct GraphInfo
{
    Qnn_GraphHandle_t graph;
    char *graphName;
    Qnn_Tensor_t *inputTensors;
    uint32_t numInputTensors;
    Qnn_Tensor_t *outputTensors;
    uint32_t numOutputTensors;
} GraphInfo_t;
typedef GraphInfo_t *GraphInfoPtr_t;

class IOTensor
{
public:
    bool setupInputAndOutputTensors(Qnn_Tensor_t **inputs,
                                    Qnn_Tensor_t **outputs,
                                    GraphInfo_t graphInfo);

    bool tearDownInputAndOutputTensors(Qnn_Tensor_t *inputs,
                                       Qnn_Tensor_t *outputs,
                                       size_t numInputTensors,
                                       size_t numOutputTensors);
    void fillDims(std::vector<size_t> &dims, uint32_t *inDimensions, uint32_t rank);
    bool writeOutputTensor(Qnn_Tensor_t *output,
                           std::vector<std::string> outputPaths,
                           std::string fileName,
                           size_t outputBatchSize);

    bool allocateAndCopyBuffer(uint8_t **buffer, Qnn_Tensor_t *tensor);

    bool tearDownTensors(Qnn_Tensor_t *tensors, uint32_t tensorCount);

    bool allocateBuffer(uint8_t **buffer, std::vector<size_t> dims, Qnn_DataType_t dataType);

    bool copyFromFloatToNative(float *floatBuffer, Qnn_Tensor_t *tensor);

    bool setupTensors(Qnn_Tensor_t **tensors, uint32_t tensorCount, Qnn_Tensor_t *tensorsInfo);

    bool deepCopyQnnTensorInfo(Qnn_Tensor_t *dst, const Qnn_Tensor_t *src);

    template <typename T>
    bool allocateBuffer(T **buffer, size_t &elementCount);
    bool convertToFloat(float **out, Qnn_Tensor_t *tensor);
};
}  // namespace vla
}  // namespace qrb