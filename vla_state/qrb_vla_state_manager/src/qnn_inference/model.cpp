/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
 
#include "model.hpp"

namespace qrb
{
namespace vla
{
static const int sg_lowerLatency = 40;    // Should be used on V66 and above only
static const int sg_lowLatency = 100;     // This will limit sleep modes available while running
static const int sg_mediumLatency = 1000; // This will limit sleep modes available while running
static const int sg_highLatency = 2000;
static std::set<uint32_t> sg_powerConfigIds = {};

uint32_t getPowerConfigId()
{
    if (sg_powerConfigIds.size() > 0)
    {
        return *sg_powerConfigIds.begin();
    }
    return 1;
}

Model::Model()
{
}

bool Model::Init_model(const std::string &model_path)
{
    //*************** (1) init qnn interface *********************/
    /*load libQNNHtp.so*/
    Backend_handle = dlopen(LIB_PATH.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (Backend_handle == nullptr)
    {
        std::cout << "[Error] Failed to dlopen lib" << std::endl;
        return false;
    }
    else
    {
        std::cout << "load lib successuly!" << std::endl;
    }

    auto get_interface_providers = loadQnnFunction<QnnInterfaceGetProvidersFn *>(Backend_handle, "QnnInterface_getProviders");
    if (get_interface_providers == nullptr)
    {
        printf("[Error] Cannot load get_providers(). dlerror: %s\n", dlerror());
        return false;
    }

    QnnInterface_t **interface_providers = nullptr;
    if (QNN_SUCCESS != get_interface_providers((const QnnInterface_t ***)&interface_providers, &Num_Providers))
    {
        std::cout << "Failed to get interface providers!" << std::endl;
        return false;
    }
    else
    {
        std::cout << "get interface successuly!" << std::endl;
    }

    if (interface_providers == nullptr || Num_Providers == 0)
    {
        std::cout << "Invalid interface providers retrieved!" << std::endl;
        return false;
    }
    std::cout << "num providers: " << Num_Providers << std::endl;

    for (size_t i = 0; i < Num_Providers; i++)
    {
        if (QNN_API_VERSION_MAJOR == interface_providers[i]->apiVersion.coreApiVersion.major && QNN_API_VERSION_MINOR <= interface_providers[i]->apiVersion.coreApiVersion.minor)
        {
            foundValidInterface = true;
            QNN_Interface = interface_providers[i]->QNN_INTERFACE_VER_NAME;
            break;
        }
    }
    // QNN_Interface = interface_providers[0]->QNN_INTERFACE_VER_NAME;

    if (foundValidInterface == false)
    {
        std::cout << "Unable to find a valid interface!" << std::endl;
        return false;
    }

    //*************** (2) init qnn system interface *********************/
    /*load libQnnSystem.so*/
    Sys_handle = dlopen(QNN_SYSLIB_PATH.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (Sys_handle == NULL)
    {
        std::cout << "[Error] Failed to dlopen sys lib" << std::endl;
        return false;
    }
    else
    {
        std::cout << "load sys lib successuly!" << std::endl;
    }

    auto get_sys_interface_providers = loadQnnFunction<QnnSystemInterfaceGetProvidersFn *>(Sys_handle, "QnnSystemInterface_getProviders");
    if (get_sys_interface_providers == nullptr)
    {
        printf("[Error] Cannot load get_sys_providers(). dlerror: %s\n", dlerror());
        return false;
    }

    QnnSystemInterface_t **sys_interface_providers = nullptr;
    uint32_t num_sys_providers = 0;
    if (QNN_SUCCESS != get_sys_interface_providers((const QnnSystemInterface_t ***)&sys_interface_providers, &num_sys_providers))
    {
        std::cout << "Failed to get sys interface providers!" << std::endl;
        return false;
    }
    else
    {
        std::cout << "get sys interface successuly!" << std::endl;
    }

    if (sys_interface_providers == nullptr || num_sys_providers == 0)
    {
        std::cout << "Invalid sys interface providers retrieved!" << std::endl;
        return false;
    }
    std::cout << "sys num providers: " << num_sys_providers << std::endl;

    bool foundValidSysInterface{false};
    for (size_t i = 0; i < num_sys_providers; i++)
    {
        if (QNN_SYSTEM_API_VERSION_MAJOR == sys_interface_providers[i]->systemApiVersion.major &&
            QNN_SYSTEM_API_VERSION_MINOR <= sys_interface_providers[i]->systemApiVersion.minor)
        {
            foundValidSysInterface = true;
        }
    }
    QNN_Sys_Interface = sys_interface_providers[0]->QNN_SYSTEM_INTERFACE_VER_NAME;
    if (foundValidSysInterface == false)
    {
        std::cout << "Unable to find a valid sys interface!" << std::endl;
        return false;
    }

    //*************** (3) init QNN backend *********************/
    const QnnBackend_Config_t *backend_0_config_0[] = {nullptr};
    auto qnnStatus_backend = QNN_Interface.backendCreate(nullptr, backend_0_config_0, &(Backend_handle)); // (loghandle, backend_0_config_0, &backend_0) we set loghandle=nullptr
    if (QNN_BACKEND_NO_ERROR != qnnStatus_backend)
    {
        std::cout << "failed to initialize backend....." << std::endl;
        return false;
    }

    //*************** (4) get DeviceProperty *********************/
    if (QNN_Interface.propertyHasCapability != nullptr)
    {
        auto qnn_status_propertyHasCapability = QNN_Interface.propertyHasCapability(QNN_PROPERTY_GROUP_DEVICE);
        if (QNN_PROPERTY_NOT_SUPPORTED == qnn_status_propertyHasCapability)
        {
            // QNN_WARN("Device property is not supported!");
            std::cout << "Warning: Device property is not supported!" << std::endl;
            std::cout << "qnn_status_propertyHasCapability:" << qnn_status_propertyHasCapability << std::endl;
            return false; // it can also be commented out
        }
        if (QNN_PROPERTY_ERROR_UNKNOWN_KEY == qnn_status_propertyHasCapability)
        {
            std::cout << "Device property is not known to backend!" << std::endl;
            return false;
        }
    }
    else
        std::cout << "QNN_Interface.propertyHasCapability == nullptr" << std::endl;
#ifdef ABC
    //*************** (5) create device *********************/
    const QnnDevice_Config_t *device_0_config_0[] = {nullptr};
    if (QNN_Interface.deviceCreate != nullptr)
    {
        auto qnnStatus_device = QNN_Interface.deviceCreate(nullptr, device_0_config_0, &Device_handle); // loghandle, device_0_config_0, &device_0
        if (qnnStatus_device != QNN_SUCCESS && qnnStatus_device != QNN_DEVICE_ERROR_UNSUPPORTED_FEATURE)
        {
            std::cout << "Failed to create device!" << std::endl;
            return false;
        }
    }
    SupportDevice = true;
    std::cout << "create device successfully!" << std::endl;
#else 
    //*************** (5) create device *********************/
    
        // *************** (5) create device *********************/
    // 直接传入 nullptr，让系统使用默认配置 (Signed PD)
    const QnnDevice_Config_t **device_0_config_0 = nullptr; 

    if (QNN_Interface.deviceCreate != nullptr)
    {
        // 注意第二个参数现在传 nullptr (或者 device_0_config_0)
        auto qnnStatus_device = QNN_Interface.deviceCreate(nullptr, nullptr, &Device_handle);
        
        if (qnnStatus_device != QNN_SUCCESS && qnnStatus_device != QNN_DEVICE_ERROR_UNSUPPORTED_FEATURE)
        {
            std::cout << "Failed to create device! Error Code: " << qnnStatus_device << std::endl;
            return false;
        }
    }
    SupportDevice = true;
    std::cout << "create device successfully!" << std::endl;

#endif
    //*************** (5) init graph from binary file *********************/
    if (nullptr == QNN_Sys_Interface.systemContextCreate ||
        nullptr == QNN_Sys_Interface.systemContextGetBinaryInfo ||
        nullptr == QNN_Sys_Interface.systemContextFree)
    {
        std::cout << "QNN System function pointers are not populated." << std::endl;
        return false;
    }

    uint64_t bufferSize{0};
    std::shared_ptr<uint8_t> buffer{nullptr};
    // read serialized binary into a byte buffer
    bufferSize = datautil::getFileSize(model_path);
    if (0 == bufferSize)
    {
        std::cout << "Received path to an empty file. Nothing to deserialize." << std::endl;
        return false;
    }
    buffer = std::shared_ptr<uint8_t>(new uint8_t[bufferSize], std::default_delete<uint8_t[]>());
    auto status = datautil::readBinaryFromFile(model_path, reinterpret_cast<uint8_t *>(buffer.get()), bufferSize);
    if (!status)
    {
        std::cout << "Could not read binary file." << std::endl;
        return false;
    }
    std::cout << "read binary file succesfully!" << std::endl;

    // inspect binary info
    QnnSystemContext_Handle_t sysCtxHandle{nullptr};
    if (QNN_SUCCESS != QNN_Sys_Interface.systemContextCreate(&sysCtxHandle))
    {
        std::cout << "Could not create system handle." << std::endl;
        return false;
    }

    const QnnSystemContext_BinaryInfo_t *binaryInfo{nullptr};
    Qnn_ContextBinarySize_t binaryInfoSize{0};
    if (
        QNN_SUCCESS != QNN_Sys_Interface.systemContextGetBinaryInfo(
                           sysCtxHandle,
                           static_cast<void *>(buffer.get()),
                           bufferSize,
                           &binaryInfo,
                           &binaryInfoSize))
    {
        std::cout << "Failed to get context binary info" << std::endl;
        return false;
    }
    // fill GraphInfo_t based on binary info
    if (!copyMetadataToGraphsInfo(binaryInfo, Q_GraphsInfo, Q_GraphsCount))
    {
        std::cout << "Failed to copy metadata." << std::endl;
        return false;
    }
    QNN_Sys_Interface.systemContextFree(sysCtxHandle);
    sysCtxHandle = nullptr;

    if (nullptr == QNN_Interface.contextCreateFromBinary)
    {
        std::cout << "contextCreateFromBinaryFnHandle is nullptr." << std::endl;
        return false;
    }
    if (
        QNN_Interface.contextCreateFromBinary(
            Backend_handle,
            Device_handle,
            nullptr,
            static_cast<void *>(buffer.get()),
            bufferSize,
            &Context,
            nullptr))
    {
        std::cout << "Could not create context from binary." << std::endl;
        return false;
    }

    for (size_t graphIdx = 0; graphIdx < Q_GraphsCount; graphIdx++)
    {
        if (nullptr == QNN_Interface.graphRetrieve)
        {
            std::cout << "graphRetrieveFnHandle is nullptr." << std::endl;
            return false;
        }
        if (QNN_Interface.graphRetrieve(Context, (*Q_GraphsInfo)[graphIdx].graphName, &((*Q_GraphsInfo)[graphIdx].graph)) != QNN_SUCCESS)
        {
            printf("Unable to retrieve graph handle for graph Idx: %d", graphIdx);
            return false;
        }
    }

    //************* (6) init performance to accelerate model*******************/
    QnnDevice_Infrastructure_t deviceInfra = nullptr;
    if (QNN_SUCCESS != QNN_Interface.deviceGetInfrastructure(&deviceInfra))
    {
        std::cout << "Failure in deviceGetInfrastructure()" << std::endl;
        return false;
    }

    QnnHtpDevice_Infrastructure_t *htpInfra = static_cast<QnnHtpDevice_Infrastructure_t *>(deviceInfra);
    Q_PerfInfra = htpInfra->perfInfra;
    uint32_t deviceId = 0;
    uint32_t coreId = 0;
    if (QNN_SUCCESS != Q_PerfInfra.createPowerConfigId(deviceId, coreId, &Q_PowerConfigId))
    {
        std::cout << "Failure in createPowerConfigId()" << std::endl;
        return false;
    }
    sg_powerConfigIds.insert(Q_PowerConfigId);
    return true;
}

void Model::Inference(std::vector<float> input_data, std::vector<std::vector<float>> &output_tensor, std::string perfProfile)
{
    std::cout << "Q_GraphsCount: " << Q_GraphsCount << std::endl;
    for (size_t graphIdx = 0; graphIdx < Q_GraphsCount; graphIdx++)
    {
        printf("Starting execution for graphIdx: %d", graphIdx);
        Qnn_Tensor_t *inputs = nullptr;
        Qnn_Tensor_t *outputs = nullptr;
        if (Q_IOTensor.setupInputAndOutputTensors(&inputs, &outputs, (*Q_GraphsInfo)[graphIdx]) == false)
        {
            printf("Error in setting up Input and output Tensors for graphIdx: %d", graphIdx);
            break;
        }
        auto graphInfo = (*Q_GraphsInfo)[graphIdx];

        printf("populateInputTensors() graphIndx %d\n", graphIdx);
        if (nullptr == inputs)
        {
            std::cout << "inputs is nullptr" << std::endl;
            return;
        }
        auto inputCount = graphInfo.numInputTensors;
        int start_index = 0;
        for (size_t inputIdx = 0; inputIdx < inputCount; inputIdx++)
        {
            size_t inputNameIdx = inputIdx;
            printf("index = %d input column index = %d ", inputIdx, inputNameIdx);
            std::string inputNodeName;
            if (QNN_TENSOR_GET_NAME(graphInfo.inputTensors[inputIdx]))
            {
                inputNodeName = QNN_TENSOR_GET_NAME(graphInfo.inputTensors[inputIdx]);
                std::cout << "inputNodeName: " << inputNodeName << std::endl;
            }

            std::vector<size_t> dims;
            auto input = &inputs[inputIdx];
            Q_IOTensor.fillDims(dims, QNN_TENSOR_GET_DIMENSIONS(input), QNN_TENSOR_GET_RANK(input));
            if (QNN_TENSOR_GET_DATA_TYPE(input) != QNN_DATATYPE_FLOAT_32)
            {
                uint8_t *fileToBuffer = nullptr;
                if (!Q_IOTensor.allocateBuffer(&fileToBuffer, dims, QNN_DATATYPE_FLOAT_32))
                {
                    std::cout << "allocate Buffer failed!" << std::endl;
                }
                if (fileToBuffer == nullptr)
                {
                    std::cout << "buffer is nullptr!" << std::endl;
                    return;
                }
                size_t tensorLength{0};
                tensorLength = datautil::calculateLength(dims, QNN_DATATYPE_FLOAT_32);
                size_t numElements = tensorLength / sizeof(float);
                std::vector<float> inputData_tmp(input_data.begin() + start_index, input_data.begin() + start_index + numElements);
                start_index = start_index + numElements;

                size_t dataSizeInBytes = inputData_tmp.size() * sizeof(float);
                std::memcpy(fileToBuffer, inputData_tmp.data(), dataSizeInBytes); // !!!

                if (!Q_IOTensor.copyFromFloatToNative(reinterpret_cast<float *>(fileToBuffer), input))
                {
                    std::cout << "copyFromFloatToNative Failed!" << std::endl;
                }
                if (nullptr != fileToBuffer)
                {
                    free(fileToBuffer);
                    fileToBuffer = nullptr;
                }
            }
            else
            {
                size_t tensorLength{0};
                tensorLength = datautil::calculateLength(dims, QNN_TENSOR_GET_DATA_TYPE(input));
                size_t numElements = tensorLength / sizeof(float);
                std::vector<float> inputData_tmp(input_data.begin() + start_index, input_data.begin() + start_index + numElements);
                start_index = start_index + numElements;
                uint8_t *buffer = static_cast<uint8_t *>(QNN_TENSOR_GET_CLIENT_BUF(input).data);

                size_t dataSizeInBytes = inputData_tmp.size() * sizeof(float);
                std::memcpy(buffer, inputData_tmp.data(), dataSizeInBytes);
            }
        }

        if (false == Q_RunInCpu && "default" != perfProfile && false == boostPerformance(Q_PerfInfra, perfProfile))
        {
            std::cout << "Performance boost failure" << std::endl;
        }
        auto start1 = std::chrono::high_resolution_clock::now();
        QNN_Interface.graphExecute(graphInfo.graph,
                                   inputs,
                                   graphInfo.numInputTensors,
                                   outputs,
                                   graphInfo.numOutputTensors,
                                   nullptr,
                                   nullptr);
        auto end1 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration1 = end1 - start1;
        std::cout << "graphExecute time cost: " << duration1.count() << " ms" << std::endl;
        if (false == Q_RunInCpu && "default" != perfProfile && false == resetPerformance(Q_PerfInfra))
        {
            std::cout << "Performance reset failure" << std::endl;
        }
        if (outputs == nullptr)
        {
            std::cout << "outputs is nullptr" << std::endl;
            return;
        }
        for (size_t outputIdx = 0; outputIdx < graphInfo.numOutputTensors; outputIdx++)
        {
            printf("Writing output for outputIdx: %d\n", outputIdx);
            auto output = &outputs[outputIdx];
            if (QNN_TENSOR_GET_DATA_TYPE(outputs[outputIdx]) == QNN_DATATYPE_FLOAT_32)
            {
                std::cout << ("Writing in output->dataType == QNN_DATATYPE_FLOAT_32") << std::endl;
                std::vector<size_t> dims;
                Q_IOTensor.fillDims(dims, QNN_TENSOR_GET_DIMENSIONS(output), QNN_TENSOR_GET_RANK(output));
                for (int i = 0; i < dims.size(); i++)
                {
                    std::cout << std::to_string(dims[i]) << std::endl;
                }
                size_t OutputLength{0};
                OutputLength = datautil::calculateLength(dims, QNN_TENSOR_GET_DATA_TYPE(output));
                size_t numElements = datautil::calculateElementCount(dims);
                float *floatBuffer = reinterpret_cast<float *>(QNN_TENSOR_GET_CLIENT_BUF(output).data);
                std::vector<float> outputVector;
                outputVector.assign(floatBuffer, floatBuffer + numElements);
                output_tensor.push_back(outputVector);
            }
            else
            {
                std::vector<size_t> dims;
                Q_IOTensor.fillDims(dims, QNN_TENSOR_GET_DIMENSIONS(output), QNN_TENSOR_GET_RANK(output));
                for (int i = 0; i < dims.size(); i++)
                {
                    std::cout << std::to_string(dims[i]) << std::endl;
                }
                size_t numElements = datautil::calculateElementCount(dims);
                float *floatBuffer = nullptr;
                bool returnStatus = Q_IOTensor.convertToFloat(&floatBuffer, output);
                if (true != returnStatus)
                {
                    std::cout << "failure in convertToFloat" << std::endl;
                    return;
                }
                std::vector<float> outputVector;
                outputVector.assign(floatBuffer, floatBuffer + numElements);
                output_tensor.push_back(outputVector);
            }
        }
    }
}

void Model::Inference_text(std::vector<int32_t> input_data, std::vector<std::vector<float>> &output_tensor, std::string perfProfile)
{
    for (size_t graphIdx = 0; graphIdx < Q_GraphsCount; graphIdx++)
    {
        printf("Starting execution for graphIdx: %d", graphIdx);
        Qnn_Tensor_t *inputs = nullptr;
        Qnn_Tensor_t *outputs = nullptr;
        if (Q_IOTensor.setupInputAndOutputTensors(&inputs, &outputs, (*Q_GraphsInfo)[graphIdx]) == false)
        {
            printf("Error in setting up Input and output Tensors for graphIdx: %d", graphIdx);
            break;
        }
        auto graphInfo = (*Q_GraphsInfo)[graphIdx];

        printf("populateInputTensors() graphIndx %d\n", graphIdx);
        if (nullptr == inputs)
        {
            std::cout << "inputs is nullptr" << std::endl;
            return;
        }
        auto inputCount = graphInfo.numInputTensors;
        for (size_t inputIdx = 0; inputIdx < inputCount; inputIdx++)
        {
            size_t inputNameIdx = inputIdx;
            printf("index = %d input column index = %d ", inputIdx, inputNameIdx);
            std::string inputNodeName;
            if (QNN_TENSOR_GET_NAME(graphInfo.inputTensors[inputIdx]))
            {
                inputNodeName = QNN_TENSOR_GET_NAME(graphInfo.inputTensors[inputIdx]);
                std::cout << "inputNodeName: " << inputNodeName << std::endl;
            }

            std::vector<size_t> dims;
            auto input = &inputs[inputIdx];
            Q_IOTensor.fillDims(dims, QNN_TENSOR_GET_DIMENSIONS(input), QNN_TENSOR_GET_RANK(input));

            size_t tensorLength{0};
            tensorLength = datautil::calculateLength(dims, QNN_TENSOR_GET_DATA_TYPE(input));

            uint8_t *buffer = static_cast<uint8_t *>(QNN_TENSOR_GET_CLIENT_BUF(input).data);

            size_t dataSizeInBytes = input_data.size() * sizeof(int32_t);
            std::memcpy(buffer, input_data.data(), dataSizeInBytes);
        }

        if (false == Q_RunInCpu && "default" != perfProfile && false == boostPerformance(Q_PerfInfra, perfProfile))
        {
            std::cout << "Performance boost failure" << std::endl;
        }

        auto start1 = std::chrono::high_resolution_clock::now();
        QNN_Interface.graphExecute(graphInfo.graph,
                                   inputs,
                                   graphInfo.numInputTensors,
                                   outputs,
                                   graphInfo.numOutputTensors,
                                   nullptr,
                                   nullptr);
        auto end1 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration1 = end1 - start1;
        std::cout << "graphExecute time cost: " << duration1.count() << " ms" << std::endl;
        if (false == Q_RunInCpu && "default" != perfProfile && false == resetPerformance(Q_PerfInfra))
        {
            std::cout << "Performance reset failure" << std::endl;
        }
        if (outputs == nullptr)
        {
            std::cout << "outputs is nullptr" << std::endl;
            return;
        }
        for (size_t outputIdx = 0; outputIdx < graphInfo.numOutputTensors; outputIdx++)
        {
            printf("Writing output for outputIdx: %d\n", outputIdx);
            auto output = &outputs[outputIdx];
            if (QNN_TENSOR_GET_DATA_TYPE(outputs[outputIdx]) == QNN_DATATYPE_FLOAT_32)
            {
                std::cout << ("Writing in output->dataType == QNN_DATATYPE_FLOAT_32") << std::endl;
                std::vector<size_t> dims;
                Q_IOTensor.fillDims(dims, QNN_TENSOR_GET_DIMENSIONS(output), QNN_TENSOR_GET_RANK(output));
                for (int i = 0; i < dims.size(); i++)
                {
                    std::cout << std::to_string(dims[i]) << std::endl;
                }
                size_t OutputLength{0};
                OutputLength = datautil::calculateLength(dims, QNN_TENSOR_GET_DATA_TYPE(output));
                size_t numElements = datautil::calculateElementCount(dims);
                float *floatBuffer = reinterpret_cast<float *>(QNN_TENSOR_GET_CLIENT_BUF(output).data);
                std::vector<float> outputVector;
                outputVector.assign(floatBuffer, floatBuffer + numElements);
                output_tensor.push_back(outputVector);
            }
            else
            {
                std::vector<size_t> dims;
                Q_IOTensor.fillDims(dims, QNN_TENSOR_GET_DIMENSIONS(output), QNN_TENSOR_GET_RANK(output));
                for (int i = 0; i < dims.size(); i++)
                {
                    std::cout << std::to_string(dims[i]) << std::endl;
                }
                size_t numElements = datautil::calculateElementCount(dims);
                float *floatBuffer = nullptr;
                bool returnStatus = Q_IOTensor.convertToFloat(&floatBuffer, output);
                if (true != returnStatus)
                {
                    std::cout << "failure in convertToFloat" << std::endl;
                    return;
                }
                std::vector<float> outputVector;
                // outputVector.assign(floatBuffer, floatBuffer + OutputLength);
                outputVector.assign(floatBuffer, floatBuffer + numElements);
                output_tensor.push_back(outputVector);
            }
        }
    }
}

bool Model::copyTensorsInfo(const Qnn_Tensor_t *tensorsInfoSrc,
                            Qnn_Tensor_t *&tensorWrappers,
                            uint32_t tensorsCount)
{
    auto returnStatus = true;
    tensorWrappers = (Qnn_Tensor_t *)calloc(tensorsCount, sizeof(Qnn_Tensor_t));
    if (nullptr == tensorWrappers)
    {
        std::cout << "Failed to allocate memory for tensorWrappers." << std::endl;
        return false;
    }
    if (returnStatus)
    {
        for (size_t tIdx = 0; tIdx < tensorsCount; tIdx++)
        {
            std::cout << "Extracting tensorInfo for tensor Idx: " << std::to_string(tIdx) << std::endl;
            tensorWrappers[tIdx] = QNN_TENSOR_INIT;
            Q_IOTensor.deepCopyQnnTensorInfo(&tensorWrappers[tIdx], &tensorsInfoSrc[tIdx]);
        }
    }
    return returnStatus;
}

bool Model::copyGraphsInfoV1(const QnnSystemContext_GraphInfoV1_t *graphInfoSrc,
                             GraphInfo_t *graphInfoDst)
{
    graphInfoDst->graphName = nullptr;
    if (graphInfoSrc->graphName)
    {
        graphInfoDst->graphName = datautil::StringOpStrndup(graphInfoSrc->graphName, strlen(graphInfoSrc->graphName));
    }
    graphInfoDst->inputTensors = nullptr;
    graphInfoDst->numInputTensors = 0;
    if (graphInfoSrc->graphInputs)
    {
        if (!copyTensorsInfo(
                graphInfoSrc->graphInputs, graphInfoDst->inputTensors, graphInfoSrc->numGraphInputs))
        {
            return false;
        }
        graphInfoDst->numInputTensors = graphInfoSrc->numGraphInputs;
    }
    graphInfoDst->outputTensors = nullptr;
    graphInfoDst->numOutputTensors = 0;
    if (graphInfoSrc->graphOutputs)
    {
        if (!copyTensorsInfo(graphInfoSrc->graphOutputs,
                             graphInfoDst->outputTensors,
                             graphInfoSrc->numGraphOutputs))
        {
            return false;
        }
        graphInfoDst->numOutputTensors = graphInfoSrc->numGraphOutputs;
    }
    return true;
}

bool Model::copyGraphsInfoV3(const QnnSystemContext_GraphInfoV3_t *graphInfoSrc,
                             GraphInfo_t *graphInfoDst)
{
    graphInfoDst->graphName = nullptr;
    if (graphInfoSrc->graphName)
    {
        graphInfoDst->graphName = datautil::StringOpStrndup(graphInfoSrc->graphName, strlen(graphInfoSrc->graphName));
    }
    graphInfoDst->inputTensors = nullptr;
    graphInfoDst->numInputTensors = 0;
    if (graphInfoSrc->graphInputs)
    {
        if (!copyTensorsInfo(
                graphInfoSrc->graphInputs, graphInfoDst->inputTensors, graphInfoSrc->numGraphInputs))
        {
            return false;
        }
        graphInfoDst->numInputTensors = graphInfoSrc->numGraphInputs;
    }
    graphInfoDst->outputTensors = nullptr;
    graphInfoDst->numOutputTensors = 0;
    if (graphInfoSrc->graphOutputs)
    {
        if (!copyTensorsInfo(graphInfoSrc->graphOutputs,
                             graphInfoDst->outputTensors,
                             graphInfoSrc->numGraphOutputs))
        {
            return false;
        }
        graphInfoDst->numOutputTensors = graphInfoSrc->numGraphOutputs;
    }
    return true;
}

bool Model::copyGraphsInfo(const QnnSystemContext_GraphInfo_t *graphsInput,
                           const uint32_t numGraphs,
                           GraphInfo_t **&graphsInfo)
{
    if (!graphsInput)
    {
        std::cout << "Received nullptr for graphsInput." << std::endl;
        return false;
    }
    auto returnStatus = true;
    graphsInfo =
        (GraphInfo_t **)calloc(numGraphs, sizeof(GraphInfo_t *));
    GraphInfo_t *graphInfoArr =
        (GraphInfo_t *)calloc(numGraphs, sizeof(GraphInfo_t));
    if (nullptr == graphsInfo || nullptr == graphInfoArr)
    {
        std::cout << "Failure to allocate memory for *graphInfo" << std::endl;
        returnStatus = false;
    }
    if (true == returnStatus)
    {
        for (size_t gIdx = 0; gIdx < numGraphs; gIdx++)
        {
            printf("Extracting graphsInfo for graph Idx: %d", gIdx);
            if (graphsInput[gIdx].version == QNN_SYSTEM_CONTEXT_GRAPH_INFO_VERSION_1)
            {
                copyGraphsInfoV1(&graphsInput[gIdx].graphInfoV1, &graphInfoArr[gIdx]);
            }
            else if (graphsInput[gIdx].version == QNN_SYSTEM_CONTEXT_GRAPH_INFO_VERSION_3)
            {
                copyGraphsInfoV3(&graphsInput[gIdx].graphInfoV3, &graphInfoArr[gIdx]);
            }
            graphsInfo[gIdx] = graphInfoArr + gIdx;
        }
    } else {
        return false;
    }

    return true;
}
bool Model::copyMetadataToGraphsInfo(const QnnSystemContext_BinaryInfo_t *binaryInfo, GraphInfo_t **&graphsInfo, uint32_t &graphsCount)
{
    if (nullptr == binaryInfo)
    {
        std::cout << "binaryInfo is nullptr." << std::endl;
        return false;
    }
    graphsCount = 0;
    if (binaryInfo->version == QNN_SYSTEM_CONTEXT_BINARY_INFO_VERSION_1)
    {
        if (binaryInfo->contextBinaryInfoV1.graphs)
        {
            if (!copyGraphsInfo(binaryInfo->contextBinaryInfoV1.graphs,
                                binaryInfo->contextBinaryInfoV1.numGraphs,
                                graphsInfo))
            {
                std::cout << "Failed while copying graphs Info." << std::endl;
                return false;
            }
            graphsCount = binaryInfo->contextBinaryInfoV1.numGraphs;
            return true;
        }
    } else if (binaryInfo->version == QNN_SYSTEM_CONTEXT_BINARY_INFO_VERSION_2)
    {
        if (binaryInfo->contextBinaryInfoV2.graphs)
        {
            if (!copyGraphsInfo(binaryInfo->contextBinaryInfoV2.graphs,
                                binaryInfo->contextBinaryInfoV2.numGraphs,
                                graphsInfo))
            {
                std::cout << "Failed while copying graphs Info." << std::endl;
                return false;
            }
            graphsCount = binaryInfo->contextBinaryInfoV2.numGraphs;
            return true;
        }
    }
    else if (binaryInfo->version == QNN_SYSTEM_CONTEXT_BINARY_INFO_VERSION_3)
    {
        if (binaryInfo->contextBinaryInfoV3.graphs)
        {
            if (!copyGraphsInfo(binaryInfo->contextBinaryInfoV3.graphs,
                                binaryInfo->contextBinaryInfoV3.numGraphs,
                                graphsInfo))
            {
                std::cout << "Failed while copying graphs Info." << std::endl;
                return false;
            }
            graphsCount = binaryInfo->contextBinaryInfoV3.numGraphs;
            return true;
        }
    }
    std::cout << "Unrecognized system context binary info version." << std::endl;
    return false;
}

bool Model::disableDcvs(QnnHtpDevice_PerfInfrastructure_t perfInfra)
{
    QnnHtpPerfInfrastructure_PowerConfig_t powerConfig;
    memset(&powerConfig, 0, sizeof(powerConfig));
    powerConfig.option = QNN_HTP_PERF_INFRASTRUCTURE_POWER_CONFIGOPTION_DCVS_V3;
    powerConfig.dcvsV3Config.dcvsEnable = 0; // FALSE
    powerConfig.dcvsV3Config.setDcvsEnable = 1;
    powerConfig.dcvsV3Config.powerMode = QNN_HTP_PERF_INFRASTRUCTURE_POWERMODE_ADJUST_UP_DOWN;
    powerConfig.dcvsV3Config.contextId = getPowerConfigId();

    const QnnHtpPerfInfrastructure_PowerConfig_t *powerConfigs[] = {&powerConfig, NULL};

    if (QNN_SUCCESS != perfInfra.setPowerConfig(getPowerConfigId(), powerConfigs))
    {
        std::cout << "Failure in setPowerConfig() from disableDcvs" << std::endl;
        return false;
    }
    return true;
}

bool Model::enableDcvs(QnnHtpDevice_PerfInfrastructure_t perfInfra)
{
    QnnHtpPerfInfrastructure_PowerConfig_t powerConfig;
    memset(&powerConfig, 0, sizeof(powerConfig));
    powerConfig.option = QNN_HTP_PERF_INFRASTRUCTURE_POWER_CONFIGOPTION_DCVS_V3;
    powerConfig.dcvsV3Config.dcvsEnable = 1;
    powerConfig.dcvsV3Config.setDcvsEnable = 1;
    powerConfig.dcvsV3Config.powerMode = QNN_HTP_PERF_INFRASTRUCTURE_POWERMODE_ADJUST_UP_DOWN;
    powerConfig.dcvsV3Config.contextId = getPowerConfigId();

    const QnnHtpPerfInfrastructure_PowerConfig_t *powerConfigs[] = {&powerConfig, NULL};

    if (QNN_SUCCESS != perfInfra.setPowerConfig(getPowerConfigId(), powerConfigs))
    {
        std::cout << "Failure in setPowerConfig() from disableDcvs" << std::endl;
        return false;
    }
    return true;
}

bool Model::boostPerformance(QnnHtpDevice_PerfInfrastructure_t perfInfra, std::string perfProfile)
{
    // Initialize the power config and select the voltage corner values for the performance setting.
    QnnHtpPerfInfrastructure_PowerConfig_t powerConfig;
    memset(&powerConfig, 0, sizeof(powerConfig));
    std::cout << "PERF::boostPerformance" << std::endl;

    powerConfig.option = QNN_HTP_PERF_INFRASTRUCTURE_POWER_CONFIGOPTION_DCVS_V3;
    powerConfig.dcvsV3Config.dcvsEnable = 1;
    powerConfig.dcvsV3Config.setDcvsEnable = 1;
    powerConfig.dcvsV3Config.contextId = getPowerConfigId();

    // refer QnnHtpPerfInfrastructure.h
    powerConfig.dcvsV3Config.powerMode = QNN_HTP_PERF_INFRASTRUCTURE_POWERMODE_PERFORMANCE_MODE;
    powerConfig.dcvsV3Config.setSleepLatency = 1; // True to consider Latency parameter otherwise False
    powerConfig.dcvsV3Config.setBusParams = 1;    // True to consider Bus parameter otherwise False
    powerConfig.dcvsV3Config.setCoreParams = 1;   // True to consider Core parameter otherwise False
    powerConfig.dcvsV3Config.sleepDisable = 1;    // True to disable sleep, False to re-enable sleep
    powerConfig.dcvsV3Config.setSleepDisable = 1; // True to consider sleep disable/enable parameter otherwise False

    if (perfProfile == "burst")
    {
        std::cout << "boostPerformance::perfProfile=burst" << std::endl;
        powerConfig.dcvsV3Config.sleepLatency = sg_lowerLatency; // set dsp sleep latency ranges 10-65535 micro sec, refer hexagon sdk;
        powerConfig.dcvsV3Config.busVoltageCornerMin = DCVS_VOLTAGE_VCORNER_MAX_VOLTAGE_CORNER;
        powerConfig.dcvsV3Config.busVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_MAX_VOLTAGE_CORNER;
        powerConfig.dcvsV3Config.busVoltageCornerMax = DCVS_VOLTAGE_VCORNER_MAX_VOLTAGE_CORNER;
        powerConfig.dcvsV3Config.coreVoltageCornerMin = DCVS_VOLTAGE_VCORNER_MAX_VOLTAGE_CORNER;
        powerConfig.dcvsV3Config.coreVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_MAX_VOLTAGE_CORNER;
        powerConfig.dcvsV3Config.coreVoltageCornerMax = DCVS_VOLTAGE_VCORNER_MAX_VOLTAGE_CORNER;
    }
    else if (perfProfile == "high_performance")
    {
        std::cout << ("boostPerformance::perfProfile=high_performance");
        powerConfig.dcvsV3Config.sleepLatency = sg_lowLatency;
        powerConfig.dcvsV3Config.busVoltageCornerMin = DCVS_VOLTAGE_VCORNER_TURBO;
        powerConfig.dcvsV3Config.busVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_TURBO;
        powerConfig.dcvsV3Config.busVoltageCornerMax = DCVS_VOLTAGE_VCORNER_TURBO;
        powerConfig.dcvsV3Config.coreVoltageCornerMin = DCVS_VOLTAGE_VCORNER_TURBO;
        powerConfig.dcvsV3Config.coreVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_TURBO;
        powerConfig.dcvsV3Config.coreVoltageCornerMax = DCVS_VOLTAGE_VCORNER_TURBO;
    }
    else
    {
        printf("Invalid performance profile %s to set power configs", perfProfile);
        return false;
    }

    // Set power config with different performance parameters
    const QnnHtpPerfInfrastructure_PowerConfig_t *powerConfigs[] = {&powerConfig, NULL};
    if (QNN_SUCCESS != perfInfra.setPowerConfig(getPowerConfigId(), powerConfigs))
    {
        std::cout << "Failure in setPowerConfig() from boostPerformance" << std::endl;
        return false;
    }

    return disableDcvs(perfInfra);
}

bool Model::resetPerformance(QnnHtpDevice_PerfInfrastructure_t perfInfra)
{
    // Initialize the power config and select the voltage corner values for the performance setting.
    QnnHtpPerfInfrastructure_PowerConfig_t powerConfig;
    memset(&powerConfig, 0, sizeof(powerConfig));

    std::cout << "PERF::resetPerformance" << std::endl;

    powerConfig.option = QNN_HTP_PERF_INFRASTRUCTURE_POWER_CONFIGOPTION_DCVS_V3;
    powerConfig.dcvsV3Config.dcvsEnable = 1;
    powerConfig.dcvsV3Config.setDcvsEnable = 1;
    powerConfig.dcvsV3Config.contextId = getPowerConfigId();
    powerConfig.dcvsV3Config.sleepLatency = sg_highLatency;
    powerConfig.dcvsV3Config.setSleepLatency = 1;
    powerConfig.dcvsV3Config.sleepDisable = 0;
    powerConfig.dcvsV3Config.setSleepDisable = 0;
    powerConfig.dcvsV3Config.powerMode = QNN_HTP_PERF_INFRASTRUCTURE_POWERMODE_POWER_SAVER_MODE;
    powerConfig.dcvsV3Config.busVoltageCornerMin = DCVS_VOLTAGE_VCORNER_MIN_VOLTAGE_CORNER;
    powerConfig.dcvsV3Config.busVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_MIN_VOLTAGE_CORNER;
    powerConfig.dcvsV3Config.busVoltageCornerMax = DCVS_VOLTAGE_VCORNER_MIN_VOLTAGE_CORNER;
    powerConfig.dcvsV3Config.setBusParams = 1;
    powerConfig.dcvsV3Config.coreVoltageCornerMin = DCVS_VOLTAGE_VCORNER_MIN_VOLTAGE_CORNER;
    powerConfig.dcvsV3Config.coreVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_MIN_VOLTAGE_CORNER;
    powerConfig.dcvsV3Config.coreVoltageCornerMax = DCVS_VOLTAGE_VCORNER_MIN_VOLTAGE_CORNER;
    powerConfig.dcvsV3Config.setCoreParams = 1;

    // Set power config with different performance parameters
    const QnnHtpPerfInfrastructure_PowerConfig_t *powerConfigs[] = {&powerConfig, NULL};
    if (QNN_SUCCESS != perfInfra.setPowerConfig(getPowerConfigId(), powerConfigs))
    {
        std::cout << "Failure in setPowerConfig() from resetPerformance" << std::endl;
        return false;
    }

    return enableDcvs(perfInfra);
}

void Model::FreeContext()
{
    if (QNN_CONTEXT_NO_ERROR != QNN_Interface.contextFree(Context, nullptr))
    {
        std::cout << "Failed to free context!" << std::endl;
    }

    Context = nullptr;
}

void Model::FreeDevice()
{
    if (true == SupportDevice)
    {
        if (nullptr != QNN_Interface.deviceFree)
        {
            auto qnn_status = QNN_Interface.deviceFree(Device_handle);
            if (QNN_SUCCESS != qnn_status && QNN_DEVICE_ERROR_UNSUPPORTED_FEATURE != qnn_status)
            {
                std::cout << "Failed to free device!" << std::endl;
            }
        }
    }

    Device_handle = nullptr;
}

void Model::FreeBackend()
{
    if (QNN_Interface.backendFree != nullptr)
    {
        if (QNN_BACKEND_NO_ERROR != QNN_Interface.backendFree(Backend_handle))
        {
            std::cout << "Could not free backend!" << std::endl;
        }
    }

    Backend_handle = nullptr;
}

Model::~Model()
{
    FreeContext();
    FreeDevice();
    FreeBackend();

    if (Sys_handle)
    {
        ::dlclose(Sys_handle);
        Sys_handle = nullptr;
    }

    if (true != Q_RunInCpu)
    {
        if (QNN_SUCCESS != Q_PerfInfra.destroyPowerConfigId(Q_PowerConfigId))
        {
            std::cout << "Failure in destroyPowerConfigId()" << std::endl;
        }
        sg_powerConfigIds.erase(Q_PowerConfigId);
    }
}
}  // namespace vla
}  // namespace qrb