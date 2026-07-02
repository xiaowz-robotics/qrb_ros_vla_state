/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#pragma once

#include <map>
#include <queue>
#include <vector>
#include <string.h>
#include "QnnTypes.h"

namespace qrb
{
namespace vla
{
namespace datautil
{

    const size_t g_bitsPerByte = 8;

    size_t getDataTypeSizeInBytes(Qnn_DataType_t dataType);

    size_t calculateLength(std::vector<size_t> dims, Qnn_DataType_t dataType);

    size_t calculateElementCount(std::vector<size_t> dims);

    size_t getFileSize(std::string file_path);

    size_t StringOpMemscpy(void *dst, size_t dstSize, const void *src, size_t copySize);

    char *StringOpStrndup(const char *source, size_t maxlen);
    /*
     * Read data in batches from vector and try to matches the model input's
     * batches. If the vector is empty while matching the batch size of model,
     * pad the remaining buffer with zeros
     * @param filePaths image paths vector
     * @param filePathsIndexOffset index offset in the vector
     * @param loopBackToStart loop the vector to fill the remaining tensor data
     * @param dims model input dimensions
     * @param dataType to create input buffer from file
     * @param buffer to fill the input image data
     *
     * @return ReadBatchDataRetType_t returns numFilesCopied and batchSize along
     * with status
     */

    bool readBinaryFromFile(std::string filePath, uint8_t *buffer, size_t bufferSize);

    template <typename T_QuantType>
    bool floatToTfN(
        T_QuantType *out, float *in, int32_t offset, float scale, size_t numElements);

    template <typename T_QuantType>
    bool tfNToFloat(
        float *out, T_QuantType *in, int32_t offset, float scale, size_t numElements);

    template <typename T_QuantType>
    bool castToFloat(float *out, T_QuantType *in, size_t numElements);

    template <typename T_QuantType>
    bool castFromFloat(T_QuantType *out, float *in, size_t numElements);

    const std::map<Qnn_DataType_t, size_t> g_dataTypeToSize = {
        {QNN_DATATYPE_INT_8, 1},
        {QNN_DATATYPE_INT_16, 2},
        {QNN_DATATYPE_INT_32, 4},
        {QNN_DATATYPE_INT_64, 8},
        {QNN_DATATYPE_UINT_8, 1},
        {QNN_DATATYPE_UINT_16, 2},
        {QNN_DATATYPE_UINT_32, 4},
        {QNN_DATATYPE_UINT_64, 8},
        {QNN_DATATYPE_FLOAT_16, 2},
        {QNN_DATATYPE_FLOAT_32, 4},
        {QNN_DATATYPE_FLOAT_64, 8},
        {QNN_DATATYPE_SFIXED_POINT_8, 1},
        {QNN_DATATYPE_SFIXED_POINT_16, 2},
        {QNN_DATATYPE_SFIXED_POINT_32, 4},
        {QNN_DATATYPE_UFIXED_POINT_8, 1},
        {QNN_DATATYPE_UFIXED_POINT_16, 2},
        {QNN_DATATYPE_UFIXED_POINT_32, 4},
        {QNN_DATATYPE_BOOL_8, 1},
    };

    std::string LoadBytesFromFile(const std::string &path);
    float cosineSimilarity(const std::vector<float> &a, const std::vector<float> &b);
} // namespace datautil
}  // namespace vla
}  // namespace qrb