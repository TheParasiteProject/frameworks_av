/*
 * Copyright (C) 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "ApexCodecs"
// #define LOG_NDEBUG 0
#include <android-base/logging.h>

#include <new>

#include <android_media_swcodec_flags.h>

#include <android-base/no_destructor.h>
#include <apex/ApexCodecs.h>
#include <apex/ApexCodecsImpl.h>
#include <apex/ApexCodecsParam.h>

// TODO: remove when we have real implementations
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

using ::android::apexcodecs::ApexComponentIntf;
using ::android::apexcodecs::ApexComponentStoreIntf;
using ::android::base::ERROR;

struct ApexCodec_Component {
    explicit ApexCodec_Component(std::unique_ptr<ApexComponentIntf> &&comp)
        : mComponent(std::move(comp)) {
    }

    ApexCodec_Status start() {
        return mComponent->start();
    }

    ApexCodec_Status flush() {
        return mComponent->flush();
    }

    ApexCodec_Status reset() {
        return mComponent->reset();
    }

private:
    std::unique_ptr<ApexComponentIntf> mComponent;
};

struct ApexCodec_ComponentStore {
    ApexCodec_ComponentStore() : mStore((ApexComponentStoreIntf *)GetApexComponentStore()) {
        if (mStore == nullptr) {
            return;
        }
        mC2Traits = mStore->listComponents();
        mTraits.reserve(mC2Traits.size());
        for (const std::shared_ptr<const C2Component::Traits> &trait : mC2Traits) {
            mTraits.push_back(ApexCodec_ComponentTraits{
                trait->name.c_str(),                // name
                trait->mediaType.c_str(),           // mediaType
                (ApexCodec_Kind)trait->kind,        // kind
                (ApexCodec_Domain)trait->domain,    // domain
            });
        }
    }

    ApexCodec_ComponentTraits *getTraits(size_t index) {
        if (mStore == nullptr) {
            return nullptr;
        }
        if (index < mTraits.size()) {
            return mTraits.data() + index;
        } else {
            return nullptr;
        }
    }

    std::unique_ptr<ApexComponentIntf> createComponent(const char *name) {
        if (mStore == nullptr) {
            return nullptr;
        }
        return mStore->createComponent(name);
    }
private:
    ApexComponentStoreIntf *mStore;
    std::vector<std::shared_ptr<const C2Component::Traits>> mC2Traits;
    std::vector<ApexCodec_ComponentTraits> mTraits;
};

ApexCodec_ComponentStore *ApexCodec_GetComponentStore() {
    ::android::base::NoDestructor<ApexCodec_ComponentStore> store;
    return store.get();
}

ApexCodec_ComponentTraits *ApexCodec_Traits_get(
        ApexCodec_ComponentStore *store, size_t index) {
    if (!android::media::swcodec::flags::apexcodecs_base()) {
        return nullptr;
    }
    return store->getTraits(index);
}

ApexCodec_Status ApexCodec_Component_create(
        ApexCodec_ComponentStore *store, const char *name, ApexCodec_Component **comp) {
    if (!android::media::swcodec::flags::apexcodecs_base()) {
        return APEXCODEC_STATUS_NOT_FOUND;
    }
    if (store == nullptr) {
        LOG(ERROR) << "ApexCodec_Component_create: store is nullptr";
        return APEXCODEC_STATUS_BAD_VALUE;
    }
    if (name == nullptr) {
        LOG(ERROR) << "ApexCodec_Component_create: name is nullptr";
        return APEXCODEC_STATUS_BAD_VALUE;
    }
    if (comp == nullptr) {
        LOG(ERROR) << "ApexCodec_Component_create: comp is nullptr";
        return APEXCODEC_STATUS_BAD_VALUE;
    }
    *comp = nullptr;
    std::unique_ptr<ApexComponentIntf> compIntf = store->createComponent(name);
    if (compIntf == nullptr) {
        return APEXCODEC_STATUS_NOT_FOUND;
    }
    *comp = new ApexCodec_Component(std::move(compIntf));
    return APEXCODEC_STATUS_OK;
}

void ApexCodec_Component_destroy(ApexCodec_Component *comp) {
    delete comp;
}

ApexCodec_Status ApexCodec_Component_start(ApexCodec_Component *comp) {
    if (comp == nullptr) {
        return APEXCODEC_STATUS_BAD_VALUE;
    }
    return comp->start();
}

ApexCodec_Status ApexCodec_Component_flush(ApexCodec_Component *comp) {
    if (comp == nullptr) {
        return APEXCODEC_STATUS_BAD_VALUE;
    }
    return comp->flush();
}

ApexCodec_Status ApexCodec_Component_reset(ApexCodec_Component *comp) {
    if (comp == nullptr) {
        return APEXCODEC_STATUS_BAD_VALUE;
    }
    return comp->reset();
}

ApexCodec_Configurable *ApexCodec_Component_getConfigurable(
        ApexCodec_Component *comp) {
    return nullptr;
}

ApexCodec_Buffer *ApexCodec_Buffer_create() {
    return nullptr;
}

void ApexCodec_Buffer_destroy(ApexCodec_Buffer *buffer) {}

void ApexCodec_Buffer_clear(ApexCodec_Buffer *buffer) {}

ApexCodec_BufferType ApexCodec_Buffer_getType(ApexCodec_Buffer *buffer) {
    return APEXCODEC_BUFFER_TYPE_EMPTY;
}

void ApexCodec_Buffer_setBufferInfo(
        ApexCodec_Buffer *buffer,
        ApexCodec_BufferFlags flags,
        uint64_t frameIndex,
        uint64_t timestampUs) {
}

ApexCodec_Status ApexCodec_Buffer_setLinearBuffer(
        ApexCodec_Buffer *buffer,
        const ApexCodec_LinearBuffer *linearBuffer) {
    return APEXCODEC_STATUS_OMITTED;
}

ApexCodec_Status ApexCodec_Buffer_setGraphicBuffer(
        ApexCodec_Buffer *buffer,
        AHardwareBuffer *graphicBuffer) {
    return APEXCODEC_STATUS_OMITTED;
}

ApexCodec_Status ApexCodec_Buffer_setConfigUpdates(
        ApexCodec_Buffer *buffer,
        const ApexCodec_LinearBuffer *configUpdates) {
    return APEXCODEC_STATUS_OMITTED;
}

ApexCodec_Status ApexCodec_Buffer_getBufferInfo(
        ApexCodec_Buffer *buffer,
        ApexCodec_BufferFlags *outFlags,
        uint64_t *outFrameIndex,
        uint64_t *outTimestampUs) {
    return APEXCODEC_STATUS_OMITTED;
}

ApexCodec_Status ApexCodec_Buffer_getLinearBuffer(
        ApexCodec_Buffer *buffer,
        ApexCodec_LinearBuffer *outLinearBuffer) {
    return APEXCODEC_STATUS_OMITTED;
}

ApexCodec_Status ApexCodec_Buffer_getGraphicBuffer(
        ApexCodec_Buffer *buffer,
        AHardwareBuffer **outGraphicBuffer) {
    return APEXCODEC_STATUS_OMITTED;
}

ApexCodec_Status ApexCodec_Buffer_getConfigUpdates(
        ApexCodec_Buffer *buffer,
        ApexCodec_LinearBuffer *outConfigUpdates,
        bool *outOwnedByClient) {
    return APEXCODEC_STATUS_OMITTED;
}

ApexCodec_Status ApexCodec_SupportedValues_getTypeAndValues(
        ApexCodec_SupportedValues *supportedValues,
        ApexCodec_SupportedValuesType *type,
        ApexCodec_SupportedValuesNumberType *numberType,
        ApexCodec_Value **values,
        uint32_t *numValues) {
    return APEXCODEC_STATUS_OMITTED;
}

void ApexCodec_SupportedValues_destroy(ApexCodec_SupportedValues *values) {}

ApexCodec_Status ApexCodec_SettingResults_getResultAtIndex(
        ApexCodec_SettingResults *results,
        size_t index,
        ApexCodec_SettingResultFailure *failure,
        ApexCodec_ParamFieldValues *field,
        ApexCodec_ParamFieldValues **conflicts,
        size_t *numConflicts) {
    return APEXCODEC_STATUS_OMITTED;
}

void ApexCodec_SettingResults_destroy(ApexCodec_SettingResults *results) {}

ApexCodec_Status ApexCodec_Component_process(
        ApexCodec_Component *comp,
        const ApexCodec_Buffer *input,
        ApexCodec_Buffer *output,
        size_t *consumed,
        size_t *produced) {
    return APEXCODEC_STATUS_OMITTED;
}

ApexCodec_Status ApexCodec_Configurable_config(
        ApexCodec_Configurable *comp,
        ApexCodec_LinearBuffer *config,
        ApexCodec_SettingResults **results) {
    return APEXCODEC_STATUS_OMITTED;
}

ApexCodec_Status ApexCodec_Configurable_query(
        ApexCodec_Configurable *comp,
        uint32_t indices[],
        size_t numIndices,
        ApexCodec_LinearBuffer *config,
        size_t *writtenOrRequired) {
    return APEXCODEC_STATUS_OMITTED;
}

ApexCodec_Status ApexCodec_ParamDescriptors_getIndices(
        ApexCodec_ParamDescriptors *descriptors,
        uint32_t **indices,
        size_t *numIndices) {
    return APEXCODEC_STATUS_OMITTED;
}

ApexCodec_Status ApexCodec_ParamDescriptors_getDescriptor(
        ApexCodec_ParamDescriptors *descriptors,
        uint32_t index,
        ApexCodec_ParamAttribute *attr,
        const char **name,
        uint32_t **dependencies,
        size_t *numDependencies) {
    return APEXCODEC_STATUS_OMITTED;
}

void ApexCodec_ParamDescriptors_destroy(ApexCodec_ParamDescriptors *descriptors) {
}

ApexCodec_Status ApexCodec_Configurable_querySupportedParams(
        ApexCodec_Configurable *comp,
        ApexCodec_ParamDescriptors **descriptors) {
    return APEXCODEC_STATUS_OMITTED;
}

ApexCodec_Status ApexCodec_Configurable_querySupportedValues(
        ApexCodec_Configurable *comp,
        ApexCodec_SupportedValuesQuery *queries,
        size_t numQueries) {
    return APEXCODEC_STATUS_OMITTED;
}

#pragma clang diagnostic pop