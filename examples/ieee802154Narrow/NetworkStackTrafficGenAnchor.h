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

#ifndef NetworkStackTrafficGenAnchor_H_
#define NetworkStackTrafficGenAnchor_H_

#include <omnetpp.h>

#include "NetwPkt_m.h"
#include "SimpleAddress.h"
#include "AppLayer.h"
#include "BaseArp.h"
#include "BaseWorldUtility.h"
#include "NetwToMacControlInfo.h"
/**
 * @brief A module to generate traffic for the NIC, used for testing purposes.
 *
 * @ingroup exampleIEEE802154Narrow
 */
class NetworkStackTrafficGenAnchor : public AppLayer
{
private:
	/** @brief Copy constructor is not allowed.
	 */
	NetworkStackTrafficGenAnchor(const NetworkStackTrafficGenAnchor&);
	/** @brief Assignment operator is not allowed.
	 */
	NetworkStackTrafficGenAnchor& operator=(const NetworkStackTrafficGenAnchor&);

public:
	enum TrafficGenMessageKinds{
        INIT_RANGING = 1,TIMER_INIT_RANGING, RANGE_REQUEST=2,RANGE_ACCEPT=3, TIMER_RANGE_ACCEPT, REQUEST_TIME_SYNC=4,
        TIMER_REQUEST_TIME_SYNC, PMU_START=5, SET_CHANNEL=101, GET_CHANNEL=102, RANGING=500
	};
	bool ack_pkt;
    int AckLength;

	simtime_t waiting_ack;

    int nodeAddr;
protected:

	int packetLength;
	simtime_t packetTime;
	double pppt;
	int burstSize;
	int remainingBurst;
	LAddress::L3Type destination;
	long direccion;
	long nbPacketDropped;
	int destino;

	BaseArp* arp;
	LAddress::L3Type myNetwAddr;

	cMessage *delayTimer;

	BaseWorldUtility* world;

public:
	NetworkStackTrafficGenAnchor()
		: AppLayer()
		, packetLength(0)
		, packetTime()
		, pppt(0)
		, burstSize(0)
		, remainingBurst(0)
		, destination()
		, nbPacketDropped(0)
		, arp(NULL)
		, myNetwAddr()
		, delayTimer(NULL)
		, world(NULL)
	{}

	virtual ~NetworkStackTrafficGenAnchor();

	virtual void initialize(int stage);

	virtual void finish();

protected:
	/** @brief Handle self messages such as timer... */
	virtual void handleSelfMsg(cMessage *msg);

	/** @brief Handle messages from lower layer */
	virtual void handleLowerMsg(cMessage *msg);

	/** @brief Handle control messages from lower layer */
	virtual void handleLowerControl(cMessage *msg);

	/** @brief Handle messages from upper layer */
	virtual void handleUpperMsg(cMessage *msg)
	{
		opp_error("NetworkStackTrafficGenAnchor has no upper layers!");
		delete msg;
	}

	/** @brief Handle control messages from upper layer */
	virtual void handleUpperControl(cMessage *msg)
	{
		opp_error("NetworkStackTrafficGenAnchor has no upper layers!");
		delete msg;
	}


	virtual void sendRangeRequest();

	virtual void sendTimeSync(int addr);

	virtual void startRanging(int state,int channel);

	virtual void setChannel(int channel);

	virtual void getChannel();

	virtual void sendRangingPkt(int anchor);
};

#endif
