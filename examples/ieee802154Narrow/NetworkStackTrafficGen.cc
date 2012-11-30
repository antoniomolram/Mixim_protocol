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

#include "ChannelPkt_m.h"



Define_Module(NetworkStackTrafficGen);

void NetworkStackTrafficGen::initialize(int stage)
{
	AppLayer::initialize(stage);

	if(stage == 0) {


	    AckLength    = 256;
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
	case TIME_RANGE_REQUEST:
	    if(ack_pkt==false)
	           {
	               EV << "Ack entregado al ANCHOR! no hace falta reenviar" << endl;
	               ack_pkt=false;
	           }else if (ack_pkt==true)
	           {
	               EV << "Necesitamos retransmitir el Range request" << endl;
	               long destAddr = msg->par("destAddr").longValue();
	               sendRangeAccept(destAddr-1);
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
                anchor_addr = pkt->getSrcAddr()-1;
                waiting_ack = 100e-3;
                delayTimerACK   = new cMessage("Timer RANGE_REQUEST", TIME_RANGE_REQUEST);
                EV<<"Lanzamos el temporizador del RANGE_REQUEST" <<endl;
                scheduleAt(simTime()+ waiting_ack, delayTimerACK);
                delayTimerACK->addPar("destAddr");
                delayTimerACK->par("destAddr").setLongValue(pkt->getSrcAddr());
                ack_pkt=true;
                EV << "Dirección del Anchor:" <<  anchor_addr <<  endl;
                sendRangeAccept(anchor_addr);          // Added -1 because anchor_addr is +1 and I don't know why! xD
        }
        break;
    case RANGE_ACCEPT:
        EV << "Mensaje RANGE_ACCEPT recibido"<< endl;
        //Enviar enviamos TIME_SYNC

        break;
    case REQUEST_TIME_SYNC:
        EV << "Mensaje TIME SYNC recibido"<< endl;
        ack_pkt=false;
        sendPMUStart( anchor_addr);

        break;

    case RangingPkt:
        EV << "Ranging Pkt recibido "<< endl;
        getChannel();
        break;
    case 102:
        EV << "Perfecto hemos recibido un paquete Ranging" << endl;
        ReplyRangingPkt(anchor_addr);
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

void NetworkStackTrafficGen::sendRangeAccept(int anchor_dir)
{
       Request_ranging *pkt = new Request_ranging("RANGE ACCEPT",RANGE_ACCEPT );
       pkt->setBitLength(ackLength);
       pkt->setSrcAddr(myNetwAddr);
       pkt->setDestAddr(anchor_dir);
       pkt->setRanging_demand(true);
       NetwToMacControlInfo::setControlInfo(pkt, LAddress::L2BROADCAST);
       EV << "Enviando confirmación de Range Accepted al anchor ->"<< anchor_dir <<endl;
       sendDown(pkt);
}

void NetworkStackTrafficGen::sendPMUStart(int anchor_dir)
{
Request_ranging *pkt = new Request_ranging("PMU Start",PMU_START );
      pkt->setBitLength(ackLength);
      pkt->setSrcAddr(myNetwAddr);
      pkt->setDestAddr(anchor_dir);
      pkt->setRanging_demand(true);
      NetwToMacControlInfo::setControlInfo(pkt, LAddress::L2BROADCAST);
      EV << "Enviando PMUStart al anchor ->"<< anchor_dir <<endl;
      pkt->setKind(5);
      sendDown(pkt);
}


void NetworkStackTrafficGen::getChannel()
{

    ChannelPkt *ChangeChannel = new ChannelPkt("Get channel",0);

    ChangeChannel->setBitLength(AckLength);
    NetwToMacControlInfo::setControlInfo(ChangeChannel, LAddress::L2BROADCAST);
    ChangeChannel->setSetChannel(false);
    sendDown(ChangeChannel);

}


void NetworkStackTrafficGen::setChannel(int channel) {
    ChannelPkt *ChangeChannel = new ChannelPkt("Set channel",0);
    ChangeChannel->setBitLength(AckLength);
    NetwToMacControlInfo::setControlInfo(ChangeChannel, LAddress::L2BROADCAST);
    ChangeChannel->setChannel(channel);
    ChangeChannel->setSetChannel(true);
    sendDown(ChangeChannel);
}

void NetworkStackTrafficGen::ReplyRangingPkt(int anchor_dir){
    Request_ranging *pkt = new Request_ranging("Ranging Reply",500 );
          pkt->setBitLength(ackLength);
          pkt->setSrcAddr(myNetwAddr);
          pkt->setDestAddr(anchor_dir);
          pkt->setRanging_demand(true);
          NetwToMacControlInfo::setControlInfo(pkt, LAddress::L2BROADCAST);
          EV << "Replicando RangingPkt al anchor:"<< anchor_dir <<endl;
          pkt->setKind(500);
          sendDown(pkt);

}
