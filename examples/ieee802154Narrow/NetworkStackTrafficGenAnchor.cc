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

#include "NetworkStackTrafficGenAnchor.h"

#include <cassert>

#include "Packet.h"
#include "request_ranging_m.h"
#include "BaseMacLayer.h"
#include "FindModule.h"
#include "NetwToMacControlInfo.h"

Define_Module(NetworkStackTrafficGenAnchor);

void NetworkStackTrafficGenAnchor::initialize(int stage)
{
	BaseLayer::initialize(stage);

	if(stage == 0) {
		world        = FindModule<BaseWorldUtility*>::findGlobalModule();
		delayTimer   = new cMessage("delay-timer", SEND_BROADCAST_TIMER);

		arp          = FindModule<BaseArp*>::findSubModule(findHost());
		myNetwAddr   = arp->myNetwAddr(this);
		direccion = 3;
		packetLength = par("packetLength");
		packetTime   = par("packetTime");
		pppt         = par("packetsPerPacketTime");
		burstSize    = par("burstSize");
		//destination  = LAddress::L3Type(par("destination").longValue());
		destination = LAddress::L3Type(direccion);
		nbPacketDropped = 0;
	} else if (stage == 1) {
		if(burstSize > 0) {
			remainingBurst = burstSize;
			scheduleAt(dblrand() * packetTime * burstSize / pppt, delayTimer);
		}
	} else {

	}
}

NetworkStackTrafficGenAnchor::~NetworkStackTrafficGenAnchor() {
	cancelAndDelete(delayTimer);
}


void NetworkStackTrafficGenAnchor::finish()
{
	recordScalar("dropped", nbPacketDropped);
}

void NetworkStackTrafficGenAnchor::handleSelfMsg(cMessage *msg)
{

	switch( msg->getKind() )
	{
	case SEND_BROADCAST_TIMER:
		assert(msg == delayTimer);
		//sendBroadcast();
		sendUnicast();
		remainingBurst--;
		if(remainingBurst == 0) {
			remainingBurst = burstSize;
			scheduleAt(simTime() + (dblrand()*1.4+0.3)*packetTime * burstSize / pppt, msg);
		} else {
			scheduleAt(simTime() + packetTime * 2, msg);
		}
		break;



	default:
		EV << "Unkown selfmessage! -> delete, kind: "<<msg->getKind() <<endl;
		delete msg;
		break;
	}
}


void NetworkStackTrafficGenAnchor::handleLowerMsg(cMessage *msg)
{
	Packet p(packetLength, 1, 0);
	emit(BaseMacLayer::catPacketSignal, &p);
	msg->getKind();
    switch( msg->getKind() )
        {
        case UNICAST_MESSAGE:
            EV << "Mensaje unicast recibido en el ancla de un nodo" << endl;
            delete msg;
            msg = 0;
        break;
        }
}


void NetworkStackTrafficGenAnchor::handleLowerControl(cMessage *msg)
{
	if(msg->getKind() == BaseMacLayer::PACKET_DROPPED) {
		nbPacketDropped++;
	}
	delete msg;
	msg = 0;
}

void NetworkStackTrafficGenAnchor::sendBroadcast()
{
	NetwPkt *pkt = new NetwPkt("BROADCAST_ANCHOR", BROADCAST_MESSAGE);
	pkt->setBitLength(packetLength);

	pkt->setSrcAddr(myNetwAddr);
	pkt->setDestAddr(destination);

	NetwToMacControlInfo::setControlInfo(pkt, LAddress::L2BROADCAST);

	Packet p(packetLength, 0, 1);
	emit(BaseMacLayer::catPacketSignal, &p);
    short kind=0; // Start request
    pkt->setKind(kind);
    pkt->addPar("parameter");
	sendDown(pkt);


}
void NetworkStackTrafficGenAnchor::sendUnicast()
{
    Request_ranging *pkt = new Request_ranging("BROADCAST_ANCHOR", UNICAST_MESSAGE);
    pkt->setBitLength(packetLength);
    pkt->setSrcAddr(myNetwAddr);
    pkt->setDestAddr(destination);
    pkt->setRanging_demand(true);
    pkt->setNombre_cualquiera("HOLA");
    short kind=0; // Start request
    pkt->setKind(kind);

    NetwToMacControlInfo::setControlInfo(pkt, LAddress::L2BROADCAST);
    //NetwToMacControlInfo::setControlInfo(pkt, LAddress::L2BROADCAST);
    EV << "Destination:" <<    direccion  << endl;
    //Packet p(packetLength, 0, 1);
   // emit(BaseMacLayer::catPacketSignal, &p);
    sendDown(pkt);
}

