//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 
package org.mixim.examples.ieee802154Narrow;

import org.mixim.modules.node.WirelessNodeBatteryNetwl;
import org.mixim.base.modules.IMobility;
import org.mixim.modules.nic.IWirelessNicUpperCtrl;
import org.mixim.base.modules.IBaseArp;
import org.mixim.base.modules.IBaseApplLayer;
import org.mixim.base.modules.IBaseNetwLayer;
module PhyMacAnchor
{
    parameters:
        string networkType = "org.mixim.examples.ieee802154Narrow.NetworkStackTrafficGenAnchor";   //type of the network layer
        string mobilityType = default("ConstSpeedMobility"); //type of the mobility module
        string arpType = default("BaseArp"); //type of address resolution module
        string nicType = "Nic802154_TI_CC2420";
        @display("bgb=210,450,white;i=device/palm;b=40,40,rect");
        int numHosts; // total number of hosts in the network

    gates:
        input radioIn; // gate for sendDirect
    submodules:
        arp: <arpType> like IBaseArp {
            @display("p=140,100;i=block/network2");
        }
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=140,30;i=block/cogwheel");
        }
        nic: <nicType> like IWirelessNicUpperCtrl {
            parameters:
                @display("p=126,389;i=block/wrxtx,#008000");
        }
        // OSI network layer
        netwl: <networkType> like IBaseNetwLayer {
            parameters:
                @display("p=70,310;i=block/layer");
        }

    connections allowunconnected:
        nic.upperLayerOut --> netwl.lowerLayerIn;
        nic.upperLayerIn <-- netwl.lowerLayerOut;
        nic.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> netwl.lowerControlIn;
        nic.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- netwl.lowerControlOut;

        radioIn --> nic.radioIn;
}

