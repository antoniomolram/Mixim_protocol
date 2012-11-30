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

#ifndef NETWORKSTACKTRAFFICGEN_H_
#define NETWORKSTACKTRAFFICGEN_H_

#include <omnetpp.h>

#include "NetwPkt_m.h"
#include "SimpleAddress.h"
#include "AppLayer.h"
#include "BaseArp.h"
#include "BaseWorldUtility.h"
#include "request_ranging_m.h"
#include "NetwToMacControlInfo.h"

#include "ChannelPkt_m.h"


/**
 * @brief A module to generate traffic for the NIC, used for testing purposes.
 *
 * @ingroup exampleIEEE802154Narrow
 */
class NetworkStackTrafficGen : public AppLayer
{
private:
	/** @brief Copy constructor is not allowed.
	 */
	NetworkStackTrafficGen(const NetworkStackTrafficGen&);
	/** @brief Assignment operator is not allowed.
	 */
	NetworkStackTrafficGen& operator=(const NetworkStackTrafficGen&);

public:
	enum TrafficGenMessageKinds{
		SEND_BROADCAST_TIMER = 1,TIME_RANGE_REQUEST, RANGE_REQUEST=2,RANGE_ACCEPT=3,REQUEST_TIME_SYNC=4,PMU_START,
		RangingPkt=10

	};
	bool ack_pkt;
    int AckLength;
    int anchor_addr;

protected:
	int packetLength;
	int ackLength;
	simtime_t packetTime;
	double pppt;
	int burstSize;
	int remainingBurst;
	LAddress::L3Type destination;

	long nbPacketDropped;


	BaseArp* arp;
	LAddress::L3Type myNetwAddr;

	cMessage *delayTimerACK;
    cMessage *delayTimer;

    simtime_t waiting_ack;


	BaseWorldUtility* world;

public:
	NetworkStackTrafficGen()
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

	virtual ~NetworkStackTrafficGen();

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
		opp_error("NetworkStackTrafficGen has no upper layers!");
		delete msg;
	}

	/** @brief Handle control messages from upper layer */
	virtual void handleUpperControl(cMessage *msg)
	{
		opp_error("NetworkStackTrafficGen has no upper layers!");
		delete msg;
	}

	/** @brief Send a broadcast message to lower layer. */
//	virtual void sendBroadcast();

	/** @brief Send a Range Accept message to lower layer */
	virtual void sendRangeAccept(int anchor_dir);

	virtual void sendPMUStart(int anchor_dir);

    virtual void setChannel(int channel);

    virtual void getChannel();

    virtual void ReplyRangingPkt(int anchor_dir);
};

#endif
