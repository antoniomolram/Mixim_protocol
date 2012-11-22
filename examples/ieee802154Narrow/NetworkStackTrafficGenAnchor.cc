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
	    ack_pkt = true;
        AckLength    = 256;

		world        = FindModule<BaseWorldUtility*>::findGlobalModule();
		delayTimer   = new cMessage("delay-timer", INIT_RANGING);

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
	case INIT_RANGING:
        assert(msg == delayTimer);
        sendUnicast();
        waiting_ack = 100e-3  ;
        delayTimer   = new cMessage("delay-timer", TIMER_INIT_RANGING);
        scheduleAt(simTime()+waiting_ack, delayTimer);
        break;

	case TIMER_INIT_RANGING:
	    if(ack_pkt==false)
	    {
            EV << "Ack entregado no hace falta reenviar" << endl;
            ack_pkt=false;
	    }else if (ack_pkt==true)
	    {
            EV << "Necesitamos retransmitir el Range request" << endl;
	        sendUnicast();
	    }


//		remainingBurst--;
//		if(remainingBurst == 0) {
//			remainingBurst = burstSize;
//			scheduleAt(simTime() + (dblrand()*1.4+0.3)*packetTime * burstSize / pppt, msg);
//		} else {
//			scheduleAt(simTime() + packetTime * 2, msg);
//		}
		break;

	default:
		EV << "Unkown selfmessage! -> delete, kind: "<<msg->getKind() <<endl;
		delete msg;
		break;
	}
}


void NetworkStackTrafficGenAnchor::handleLowerMsg(cMessage *msg)
{
    Request_ranging *pkt = (Request_ranging *)msg;
	Packet p(packetLength, 1, 0);
	emit(BaseMacLayer::catPacketSignal, &p);
	msg->getKind();
    switch( msg->getKind() )
        {
        case RANGE_ACCEPT:
            if(pkt->getRanging_demand()){
                EV << "Perfect, aceptamos el ranging! :D" << endl;

                ack_pkt=false;
                //Confirmamos al ancla que hacemos ranging.
                sendAckNode(pkt->getSrcAddr());
            }
            else
            {
                EV << "Lo sentimos, el nodo no acepta el ranging! :(" << endl;

            }

            delete msg;
            msg = 0;
        break;


        }
}


void NetworkStackTrafficGenAnchor::handleLowerControl(cMessage *msg)
{
	if(msg->getKind() == BaseMacLayer::PACKET_DROPPED) {
		nbPacketDropped++;
        EV << "Mensaje no ha llegado"  <<endl;
	}
	delete msg;
	msg = 0;
}

void NetworkStackTrafficGenAnchor::sendUnicast()
{
    Request_ranging *pkt = new Request_ranging("RANGE REQUEST");
    pkt->setBitLength(packetLength);
    pkt->setSrcAddr(myNetwAddr);
    pkt->setDestAddr(18);
    pkt->setRanging_demand(true);
    short kind=2; // Start request
    pkt->setKind(kind);

    NetwToMacControlInfo::setControlInfo(pkt, LAddress::L2BROADCAST);

    sendDown(pkt);
}

void NetworkStackTrafficGenAnchor::sendAckNode(int addr)
{
    EV<< "Dirección del nodo a contestar ->" << addr << endl;
    Request_ranging *pkt = new Request_ranging("RANGE ACCEPT NODE ACK");
       pkt->setBitLength(AckLength);
       pkt->setSrcAddr(myNetwAddr);
       pkt->setDestAddr(addr-1);
       pkt->setRanging_demand(true);
       short kind=3;
       pkt->setKind(kind);
       NetwToMacControlInfo::setControlInfo(pkt, LAddress::L2BROADCAST);
       sendDown(pkt);
}

