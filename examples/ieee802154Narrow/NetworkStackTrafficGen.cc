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

#include "NetworkStackTrafficGen.h"

#include <cassert>

#include "Packet.h"
#include "BaseMacLayer.h"
#include "FindModule.h"
#include "NetwToMacControlInfo.h"
#include "request_ranging_m.h"


Define_Module(NetworkStackTrafficGen);

void NetworkStackTrafficGen::initialize(int stage)
{
	BaseLayer::initialize(stage);

	if(stage == 0) {
		world        = FindModule<BaseWorldUtility*>::findGlobalModule();
		delayTimer   = new cMessage("delay-timer", SEND_BROADCAST_TIMER);

		arp          = FindModule<BaseArp*>::findSubModule(findHost());
		myNetwAddr   = arp->myNetwAddr(this);

		packetLength = par("packetLength");
		ackLength    = 256;
		packetTime   = par("packetTime");
		pppt         = par("packetsPerPacketTime");
		burstSize    = par("burstSize");
		destination  = LAddress::L3Type(par("destination").longValue());

		nbPacketDropped = 0;
	} else if (stage == 1) {
		if(burstSize > 0) {
			remainingBurst = burstSize;
			scheduleAt(dblrand() * packetTime * burstSize / pppt, delayTimer);
		}
	} else {

	}
}

NetworkStackTrafficGen::~NetworkStackTrafficGen() {
	cancelAndDelete(delayTimer);
}


void NetworkStackTrafficGen::finish()
{
	recordScalar("dropped", nbPacketDropped);
}

void NetworkStackTrafficGen::handleSelfMsg(cMessage *msg)
{
	switch( msg->getKind() )
	{
	case SEND_BROADCAST_TIMER:
		assert(msg == delayTimer);
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


void NetworkStackTrafficGen::handleLowerMsg(cMessage *msg)
{
    Request_ranging *pkt = (Request_ranging *)msg;
    short kind = pkt->getKind();
    switch( kind )
        {
    case RANGE_REQUEST: // Anchor pregunta si queremos hacer Ranging.
        EV << "Solicitud de ranging" << endl;
        if(pkt->getRanging_demand())
        {
            //Comprobar más datos de paquete
            //Enviar Range Accept
            EV << "Dirección del Anchor:" <<  pkt->getSrcAddr() <<  endl;
            int anchor_addr = pkt->getSrcAddr();

            // Added -1 because anchor_addr is +1 and I don't know why! xD
            sendRangeAccept(anchor_addr-1);

        }
        break;
    case 3:
        EV << "Ack del anchor recibido"<< endl;
        break;
    default:
        EV << "Unkown received message! -> delete, kind: "<<msg->getKind() <<endl;
        delete msg;
        break;
        }
	Packet p(packetLength, 1, 0);
	emit(BaseMacLayer::catPacketSignal, &p);

	delete msg;
	msg = 0;
}


void NetworkStackTrafficGen::handleLowerControl(cMessage *msg)
{
	if(msg->getKind() == BaseMacLayer::PACKET_DROPPED) {
		nbPacketDropped++;
	}
	delete msg;
	msg = 0;
}

//void NetworkStackTrafficGen::sendBroadcast()
//{
//	NetwPkt *pkt = new NetwPkt("BROADCAST_MESSAGE", BROADCAST_MESSAGE);
//	pkt->setBitLength(packetLength);
//
//	pkt->setSrcAddr(myNetwAddr);
//	pkt->setDestAddr(destination);
//	NetwToMacControlInfo::setControlInfo(pkt, LAddress::L2BROADCAST);
//
//	Packet p(packetLength, 0, 1);
//	emit(BaseMacLayer::catPacketSignal, &p);
//
//	sendDown(pkt);
//}
void NetworkStackTrafficGen::sendRangeAccept(int anchor_dir)
{
       Request_ranging *pkt = new Request_ranging("RANGE ACCEPT ANCHOR ACK",RANGE_ACCEPT );

       pkt->setBitLength(ackLength);
       pkt->setSrcAddr(myNetwAddr);
       pkt->setDestAddr(anchor_dir);
       pkt->setRanging_demand(true);
       NetwToMacControlInfo::setControlInfo(pkt, LAddress::L2BROADCAST);
       EV << "Enviando confirmación de Range Accepted al anchor ->"<< anchor_dir <<endl;
       sendDown(pkt);
}
