/*
 * DeciderUWBIRED.h
 * Author: Jerome Rousselot <jerome.rousselot@csem.ch>
 * Copyright: (C) 2008-2010 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 *              Wireless Embedded Systems
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 */

#ifndef _UWBIRENERGYDETECTIONDECIDERV2_H
#define	_UWBIRENERGYDETECTIONDECIDERV2_H

#include <vector>
#include <map>

#include "MiXiMDefs.h"
#include "Signal_.h"
#include "Mapping.h"
#include "AirFrame_m.h"
#include "Decider.h"
#include "IEEE802154A.h"
#include "UWBIRPacket.h"
#include "MacToPhyInterface.h"

class PhyLayerUWBIR;

/**
 * @brief  This class implements a model of an energy detection receiver
 * that demodulates UWB-IR burst position modulation as defined
 * in the IEEE802154A standard (mandatory mode, high PRF).
 *
 *  The code modeling the frame clock synchronization is implemented
 *  in attemptSync(). Simply subclass this class and redefine attemptSync()
 *  if you wish to consider more sophisticated synchronization models.
 *
 *  To implement a coherent receiver, the easiest way to start is to copy-paste
 *  this code into a new class and rename it accordingly. Then, redefine
 *  decodePacket().
 *
 * Citation of the following publication is appreciated if you use the MiXiM UWB PHY model
 * for a publication of your own.
 * J. Rousselot, J.-D. Decotignie, An ultra-wideband impulse radio PHY
 * layer model for network simulation. SIMULATION January 2011 vol. 87 no. 1-2 82-112.
 *
 * For more information, see also:
 *
 * [1] J. Rousselot, J.-D. Decotignie, An ultra-wideband impulse radio PHY
 * layer model for network simulation. SIMULATION January 2011 vol. 87 no.
 * 1-2 82-112. http://dx.doi.org/10.1177/0037549710377767
 * [2] J. Rousselot, Ultra Low Power Communication Protocols for UWB
 * Impulse Radio Wireless Sensor Networks. EPFL Thesis 4720, 2010.
 * http://infoscience.epfl.ch/record/147987
 * [3]  A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
 * for Network Simulation, Jérôme Rousselot, Jean-Dominique Decotignie,
 * Second International Omnet++ Workshop,Simu'TOOLS, Rome, 6 Mar 09.
 * http://portal.acm.org/citation.cfm?id=1537714
 *
 *
 * @ingroup ieee802154a
 * @ingroup decider
*/
class MIXIM_API DeciderUWBIRED: public Decider {
private:
	bool trace, stats;
	long nbRandomBits;
	long nbFailedSyncs, nbSuccessfulSyncs;
	double nbSymbols, allThresholds;
	double vsignal2, vnoise2, snirs, snirEvals, pulseSnrs;
	double packetSNIR, packetSignal, packetNoise, packetSamples;
	IEEE802154A::config cfg;
	double snrLastPacket; /**@brief Stores the snr value of the last packet seen (see decodePacket) */
protected:
	typedef std::map<Signal*, int> tSignalMap;
	double syncThreshold;
	bool syncAlwaysSucceeds;
	bool channelSensing;
	bool synced;
	bool alwaysFailOnDataInterference;
	UWBIRPacket packet;
	double epulseAggregate, enoiseAggregate;
	tSignalMap currentSignals;
	cOutVector receivedPulses;
	cOutVector syncThresholds;
	PhyLayerUWBIR* uwbiface;
	Signal* tracking;
	int nbFramesWithInterference, nbFramesWithoutInterference;
	int nbCancelReceptions;
	int nbFinishTrackingFrames;
	int nbFinishNoiseFrames;

	enum {
		FIRST, HEADER_OVER, SIGNAL_OVER
	};
	std::vector<ConstMapping*> receivingPowers;
	ConstMapping* signalPower; // = signal->getReceivingPower();
	// store relative offsets between signals starts
	std::vector<simtime_t> offsets;
	AirFrameVector airFrameVector;
	// Create an iterator for each potentially colliding airframe
	AirFrameVector::iterator airFrameIter;

	typedef ConcatConstMapping<std::multiplies<double> > MultipliedMapping;

public:
	/** @brief Signal for emitting UWBIR packets. */
	const static simsignalwrap_t catUWBIRPacketSignal;
	const static double          noiseVariance; // P=-116.9 dBW // 404.34E-12;   v²=s²=4kb T R B (T=293 K)
	const static double          peakPulsePower; //1.3E-3 W peak power of pulse to reach  0dBm during burst; // peak instantaneous power of the transmitted pulse (A=0.6V) : 7E-3 W. But peak limit is 0 dBm

	DeciderUWBIRED(DeciderToPhyInterface* iface,
			PhyLayerUWBIR* _uwbiface,
			double _syncThreshold, bool _syncAlwaysSucceeds, bool _stats,
	               bool _trace, bool alwaysFailOnDataInterference=false)
		: Decider(iface)
		, trace(_trace), stats(_stats)
		, nbRandomBits(0)
		, nbFailedSyncs(0), nbSuccessfulSyncs(0)
		, nbSymbols(0), allThresholds(0)
		, vsignal2(0), vnoise2(0), snirs(0), snirEvals(0), pulseSnrs(0)
		, packetSNIR(0), packetSignal(0), packetNoise(0), packetSamples(0)
		, cfg()
		, snrLastPacket(0)
		, syncThreshold(_syncThreshold)
		, syncAlwaysSucceeds(_syncAlwaysSucceeds)
		, channelSensing(false)
		, synced(false)
		, alwaysFailOnDataInterference(alwaysFailOnDataInterference)
		, packet()
		, epulseAggregate(0), enoiseAggregate(0)
		, currentSignals()
		, receivedPulses()
		, syncThresholds()
		, uwbiface(_uwbiface)
		, tracking(NULL)
		, nbFramesWithInterference(0), nbFramesWithoutInterference(0)
		, nbCancelReceptions(0)
		, nbFinishTrackingFrames(0)
		, nbFinishNoiseFrames(0)
		, receivingPowers()
		, signalPower(NULL)
		, offsets()
		, airFrameVector()
		, airFrameIter()
	{
		receivedPulses.setName("receivedPulses");
		syncThresholds.setName("syncThresholds");
	}

	virtual simtime_t processSignal(AirFrame* frame);

	double getAvgThreshold() const {
		if (nbSymbols > 0)
			return allThresholds / nbSymbols;
		else
			return 0;
	};

	double getNoiseValue() const {
		 return normal(0, sqrt(noiseVariance));
	}

	void cancelReception();

	void finish();

	/**@brief Control message kinds specific to DeciderUWBIRED. Currently defines a
	 * message kind that informs the MAC of a successful SYNC event at PHY layer. */
	enum UWBIRED_CTRL_KIND {
	    SYNC_SUCCESS=MacToPhyInterface::LAST_BASE_PHY_KIND+1,
	    SYNC_FAILURE,
	    // add other control messages kinds here (from decider to mac, e.g. CCA)
	};

	// compatibility function to allow running MAC layers that depend on channel state information
	// from PHY layer. Returns last SNR
	virtual ChannelState getChannelState() const;

protected:
	bool decodePacket(Signal* signal, std::vector<bool> * receivedBits);
	simtime_t handleNewSignal(Signal* s);
	simtime_t handleHeaderOver(tSignalMap::iterator& it);
	virtual bool attemptSync(Signal* signal);
	simtime_t
			handleSignalOver(tSignalMap::iterator& it, AirFrame* frame);
	// first value is energy from signal, other value is total window energy
	std::pair<double, double> integrateWindow(int symbol, simtime_t_cref now,
			simtime_t_cref burst, Signal* signal);

	simtime_t handleChannelSenseRequest(ChannelSenseRequest* request);
private:
	/** @brief Copy constructor is not allowed.
	 */
	DeciderUWBIRED(const DeciderUWBIRED&);
	/** @brief Assignment operator is not allowed.
	 */
	DeciderUWBIRED& operator=(const DeciderUWBIRED&);

};

#endif	/* _UWBIRENERGYDETECTIONDECIDERV2_H */

