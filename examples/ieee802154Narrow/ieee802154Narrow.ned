package org.mixim.examples.ieee802154Narrow;

import org.mixim.base.modules.BaseNetwork;
import org.mixim.modules.news_sources.PhyMacAnchor;
import org.mixim.base.connectionManager.IConnectionManager;
import org.mixim.base.modules.IWorldUtility;
network ieee802154Narrow
{
    parameters:
        int numHosts; // total number of hosts in the network
        int numAnchors; // total number of anchors in the network
        //wuType = "WorldUtilityStats";
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        string cmType = default("org.mixim.base.connectionManager.ConnectionManager"); // connection manager to use
        string wuType = default("org.mixim.base.modules.BaseWorldUtility");            // world utility to use

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white,,;bgp=10,50");
    submodules:
        node[numHosts]: PhyMacHost {
            parameters:
                numHosts = numHosts;
                @display("p=86,89;b=42,42,rect,red;i=device/pocketpc");

        }
        anchor[numAnchors]: PhyMacAnchor {
            parameters:
                numHosts = numAnchors;
                @display("p=248,160;b=42,42,rect,green;i=device/accesspoint");
        }

        connectionManager: <cmType> like IConnectionManager {
            parameters:
                @display("p=201,0;i=abstract/multicast;is=s");
        }
        world: <wuType> like IWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=280,0;i=misc/globe;is=s");
        }
    connections allowunconnected:
                         // all connections and gates are to be generated dynamically

}

