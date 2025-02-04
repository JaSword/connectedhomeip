/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/****************************************************************************
 * @file
 * @brief Implementation for the Descriptor Server Cluster
 ***************************************************************************/

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerSourceConfiguration::Attributes;

namespace {

class PowerSourceConfigurationAttrAccess : public AttributeAccessInterface
{
public:
    // Register on all endpoints.
    PowerSourceConfigurationAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), PowerSourceConfiguration::Id)
    {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

PowerSourceConfigurationAttrAccess gAttrAccess;

CHIP_ERROR PowerSourceConfigurationAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aPath.mClusterId != PowerSourceConfiguration::Id || aPath.mEndpointId != 0x00)
    {
        return CHIP_ERROR_INVALID_PATH_LIST;
    }

    switch (aPath.mAttributeId)
    {
    case Sources::Id:
        err = aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
            uint16_t clusterCount = 0;

            for (uint16_t endpointIndex = 0; endpointIndex < emberAfEndpointCount(); endpointIndex++)
            {
                clusterCount = emberAfClusterCount(endpointIndex, true);

                for (uint8_t clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
                {
                    EmberAfCluster * cluster = emberAfGetNthCluster(endpointIndex, clusterIndex, true);

                    if (cluster->clusterId == PowerSource::Id)
                    {
                        ReturnErrorOnFailure(encoder.Encode(endpointIndex));
                        break; // There is only 1 server cluster per endpoint
                    }
                }
            }

            return CHIP_NO_ERROR;
        });
        break;
    default:
        break;
    }

    return err;
}

} // anonymous namespace

void MatterPowerSourceConfigurationPluginServerInitCallback(void)
{
    registerAttributeAccessOverride(&gAttrAccess);
}
