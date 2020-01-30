#ifndef PCMBasic_H_
#define PCMBasic_H_

#include "/home/abasak/pcm/utils.h"
#include "/home/abasak/pcm/cpucounters.h"
#include <cassert>

template <class CounterStateType>
void display_processor_level_stats(std::ofstream& out, const PCM * m, const CounterStateType & state1, const CounterStateType & state2){
    assert(out.is_open());
    /* Output format/order: L2 MPKI, L3 MPKI, L2 hit rate, LLC hit rate, TLP (active cycles method), TLP (C state method = BigBench paper's method)
     incoming QPI link utilization, outgoing QPI link utilization */
    assert(m->isL2CacheMissesAvailable() && 
           m->isL3CacheMissesAvailable() && 
           m->isL2CacheHitRatioAvailable() && 
           m->isL3CacheHitRatioAvailable());
    
    const uint32 qpiLinks = (uint32)m->getQPILinksPerSocket();    
    double incoming = 0.0; double outgoing = 0.0;

    for(uint32 i = 0; i < m->getNumSockets(); ++i){
        for (uint32 l = 0; l < qpiLinks; ++l){
            incoming += 100. * getIncomingQPILinkUtilization(i, l, state1, state2);
            outgoing += 100. * getOutgoingQPILinkUtilization(i, l, state1, state2);
        }
    }

    double overallIncomingQPILinkUitlization = incoming / (m->getNumSockets() * qpiLinks);
    double overallOutgoingQPILinkUitlization = outgoing / (m->getNumSockets() * qpiLinks);

    out << (double(getL2CacheMisses(state1, state2)) / getInstructionsRetired(state1, state2)) * 1000 << "," 
    << (double(getL3CacheMisses(state1, state2)) / getInstructionsRetired(state1, state2)) * 1000 << ","
    << getL2CacheHitRatio(state1, state2) << ","
    << getL3CacheHitRatio(state1, state2) << ","   
    << (double(getCycles(state1, state2))/getInvariantTSC(state1, state2)) * 100  << ","
    << getCoreCStateResidency(0, state1, state2)*100. << ","
    << overallIncomingQPILinkUitlization << ","
    << overallOutgoingQPILinkUitlization << "\n";
}

template <class CounterStateType>
void display_processor(const PCM * m, const CounterStateType & state1, const CounterStateType & state2, bool update){
    ofstream out;

    if(update){
        out.open("UpdateProcLevel.csv", std::ios_base::app); 
        display_processor_level_stats(out, m, state1, state2); 
    }
    else{
        out.open("AlgProcLevel.csv", std::ios_base::app); 
        display_processor_level_stats(out, m, state1, state2);
    }

    out.close();   
}

#endif //PCMBasic_H_