#include "AppLayer.h"
#include "NetwControlInfo.h"
#include <cassert>
#include <Packet.h>
#include <BaseMacLayer.h>

#include "FindModule.h"

void AppLayer::initialize(int stage)
{
    BaseLayer::initialize(stage);

    if(stage == 0) {
        myNetwAddr = getParentModule()->findSubmodule("nic");
       EV << "Mi dirección de red de App Layer: " << myNetwAddr << endl;
       canal=0;
    }
}
