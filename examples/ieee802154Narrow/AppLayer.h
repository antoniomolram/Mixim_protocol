#ifndef APPLAYER_H_
#define APPLAYER_H_

#include "BaseLayer.h"
#include "ApplPkt_m.h"
#include "SimpleAddress.h"
#include "BaseArp.h"
#include <BaseWorldUtility.h>
#include "BaseConnectionManager.h"
#include "NetwControlInfo.h"
#include <cassert>
#include <Packet.h>
#include <BaseMacLayer.h>
#include <omnetpp.h>


class AppLayer : public BaseLayer
{
public:

    enum TrafficGenMessageKinds{        // All the possible types of messages/tasks to send/schedule

        SEND_SYNC_TIMER_WITH_CSMA = 1,  // Used to schedule the next message
        SYNC_MESSAGE_WITH_CSMA,         // Used by the Mobile Nodes type 3 and 4 to send their broadcasts
        SEND_SYNC_TIMER_WITHOUT_CSMA,   // Used to schedule the next message
        SYNC_MESSAGE_WITHOUT_CSMA,      // Used by the Anchors to send the sync messages
        SEND_REPORT_WITH_CSMA,          // Used to schedule the next message
        REPORT_WITH_CSMA,               // Report from any host with CSMA enabled
        SEND_REPORT_WITHOUT_CSMA,       // Used to schedule the next message
        REPORT_WITHOUT_CSMA,            // Report from any host with CSMA disabled
        CALCULATE_POSITION,             // Event to simulate the processing time of the Mobile Node type 2 when it calculates its position
        CHECK_QUEUE,                    // Event to check the transmission queues from Anchors (Com Sink 1) and Computer (Com Sink 2)
        WAITING_REQUEST,                // Event to simulate the waiting time since we send the request till we receive the answer from the Anchor
        BEGIN_PHASE,                    // Event to be executed at the beginning of every phase
        WAKE_UP,                        // Event to wake up the node timeSleepToRX before activity
        SLEEP,                          // Event to sleep the node
        MAC_ERROR_MANAGEMENT            // Event to to delay the handle of packets with MAC error.
    };


protected:

    BaseArp* arp;                       // Pointer to the ARP of the Host
    int myNetwAddr;                     // Network address of the Host

    BaseConnectionManager* cc;          // Pointer to the Propagation Model module

    //Mis variables



public:

    virtual void initialize(int stage);
    int canal;
};

#endif
