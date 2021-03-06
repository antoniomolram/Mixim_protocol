/* -*- mode:c++ -*- ********************************************************
 * file:        StateChanger.h
 *
 * author:      Andreas Koepke
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     testsuite of framework
 * description: changes host state
 ***************************************************************************
 * changelog:   $Revision: 1.3 $
 *              last modified:   $Date: 2004/02/09 13:59:33 $
 *              by:              $Author: omfw-willkomm $
 **************************************************************************/


#ifndef STATE_CHANGER_H
#define STATE_CHANGER_H

#include <omnetpp.h>

#include "TestHostState.h"
#include "TestParam.h"
#include "YetAnother.h"
#include "BaseModule.h"

class StateChanger : public BaseModule
{
private:
	/** @brief Copy constructor is not allowed.
	 */
	StateChanger(const StateChanger&);
	/** @brief Assignment operator is not allowed.
	 */
	StateChanger& operator=(const StateChanger&);

public:
    const static simsignalwrap_t catHostState;
    const static simsignalwrap_t catTestParam;

private:
    cMessage *change_timer;
    unsigned int state_counter;
    TestHostState hs;
    YetAnother tp;

public:
    StateChanger()
    	: BaseModule()
    	, change_timer(NULL)
    	, state_counter(0)
    	, hs()
    	, tp()
    {}

    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(cMessage*);
};

#endif

