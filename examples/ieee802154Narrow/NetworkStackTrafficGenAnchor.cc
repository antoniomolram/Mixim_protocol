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
#include "ChannelPkt_m.h"

#include "BaseMacLayer.h"
#include "FindModule.h"
#include "NetwToMacControlInfo.h"


Define_Module(NetworkStackTrafficGenAnchor);

void NetworkStackTrafficGenAnchor::initialize(int stage)
{
	AppLayer::initialize(stage);

	if(stage == 0) {
	    ack_pkt = true;

	    nodeAddr=-1;
        AckLength    = 256;

		world        = FindModule<BaseWorldUtility*>::findGlobalModule();
		delayTimer   = new cMessage("Init pkt", INIT_RANGING);

		arp          = FindModule<BaseArp*>::findSubModule(findHost());
		myNetwAddr   = arp->myNetwAddr(this);
		packetLength = par("packetLength");
		packetTime   = par("packetTime");
		pppt         = par("packetsPerPacketTime");
		burstSize    = par("burstSize");
		destination = LAddress::L3Type(direccion);
		nbPacketDropped = 0;
	} else if (stage == 1) {
		if(burstSize > 0) {
			scheduleAt(simTime(), delayTimer);
		}
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
        sendRangeRequest();
        waiting_ack = 100e-3  ;
        delayTimer   = new cMessage("Timer request range", TIMER_INIT_RANGING);
        scheduleAt(simTime()+waiting_ack, delayTimer);
        break;

	case TIMER_INIT_RANGING:
	    if(ack_pkt==false)
	    {
            EV << "Ack entregado al NODO! no hace falta reenviar" << endl;
            ack_pkt=false;
	    }else if (ack_pkt==true)
	    {
            EV << "Necesitamos retransmitir el Range request" << endl;
	        sendRangeRequest();
	    }


		break;
	case TIMER_REQUEST_TIME_SYNC:
	    if(ack_pkt==false)
	            {
	                EV << "TIME SYNC entregado al NODO! no hace falta reenviar" << endl;
	                ack_pkt=false;
	            }else if (ack_pkt==true)
	            {
	                EV << "Necesitamos retransmitir el TIME SYNC" << endl;
	                long destAddr = msg->par("destAddr").longValue();
	                sendTimeSync(destAddr-1);
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
    Request_ranging *pkt = (Request_ranging *)msg;

    ChannelPkt *changeChannel = (ChannelPkt *)msg;
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
                nodeAddr=pkt->getSrcAddr();
                sendTimeSync(pkt->getSrcAddr());
                delayTimer   = new cMessage("TIMER TIME SYNC", TIMER_REQUEST_TIME_SYNC);
                scheduleAt(simTime()+waiting_ack, delayTimer);
                delayTimer->addPar("destAddr");
                delayTimer->par("destAddr").setLongValue(pkt->getSrcAddr());
            }else            {
                EV << "Lo sentimos, el nodo no acepta el ranging! :(" << endl;
            }
            delete msg;
            msg = 0;
        break;

        case PMU_START:
            EV << "RANGING MEASUREMENT" << endl;
            startRanging(0,0);
            break;

        case SET_CHANNEL:
            EV << "Contestación SetChannel" << endl;

            //changeChannel->getChannel();
            EV << "Canal actualizado" <<  changeChannel->getChannel() << endl;


        break;
        case GET_CHANNEL:
            EV << "Contestación GetChannel" << endl;
            //changeChannel->getChannel();
            sendRangingPkt(nodeAddr);
            //startRanging(1, changeChannel->getChannel());
        break;
        case RANGING:




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

void NetworkStackTrafficGenAnchor::sendRangeRequest()
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

void NetworkStackTrafficGenAnchor::sendTimeSync(int addr)
{
    EV<< "Dirección del nodo a contestar ->" << addr << endl;
    Request_ranging *pkt = new Request_ranging("TIME SYNC");
       pkt->setBitLength(AckLength);
       pkt->setSrcAddr(myNetwAddr);
       pkt->setDestAddr(addr-1);
       pkt->setRanging_demand(true);
       short kind=4;
       pkt->setKind(kind);
       NetwToMacControlInfo::setControlInfo(pkt, LAddress::L2BROADCAST);
       sendDown(pkt);
}

void NetworkStackTrafficGenAnchor::startRanging(int state, int channel){



    //Funcion cambiar channel
    //NetworkStackTrafficGenAnchor::setChannel(5);

    //Funcion getchannel

    /*
     * States
     *
     * 1º INIT => state=0
     * 2º CHANNEL_JUMP  state=1
     * 3º END =>  state=3
     *
     *
     */

    switch(state)
    {
    case 0: // Init
        NetworkStackTrafficGenAnchor::getChannel();

    break;
    case 1:
            if (channel<=79){
                setChannel(channel+1);
            }
        break;
    }

    /*
     * Algoritmo
     * iniciamos
     *  1º pedimos el canal ==> Function getchannel
     *  si el canal es el 0 iniciamos contadores
     *  si no canal=canal+1 ==> Function setchannel
     * hasta que canal=80
     *
     *
     */


}

void NetworkStackTrafficGenAnchor::getChannel()
{
    ChannelPkt *ChangeChannel = new ChannelPkt("Get channel",0);
    ChangeChannel->setBitLength(AckLength);
    NetwToMacControlInfo::setControlInfo(ChangeChannel, LAddress::L2BROADCAST);
    ChangeChannel->setSetChannel(false);
    sendDown(ChangeChannel);

}


void NetworkStackTrafficGenAnchor::setChannel(int channel) {
    ChannelPkt *ChangeChannel = new ChannelPkt("Set channel",0);
    ChangeChannel->setBitLength(AckLength);
    NetwToMacControlInfo::setControlInfo(ChangeChannel, LAddress::L2BROADCAST);
    ChangeChannel->setChannel(channel);
    ChangeChannel->setSetChannel(true);
    sendDown(ChangeChannel);
}

void NetworkStackTrafficGenAnchor::sendRangingPkt(int anchor)
{
      Request_ranging *pkt = new Request_ranging("RangingPkt");
         pkt->setBitLength(10);
         pkt->setSrcAddr(myNetwAddr);
         pkt->setDestAddr(anchor-1);

         short kind=10;
         pkt->setKind(kind);
         NetwToMacControlInfo::setControlInfo(pkt, LAddress::L2BROADCAST);
         sendDown(pkt);
}
