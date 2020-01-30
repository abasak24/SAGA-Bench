#ifndef PCMMemory_H_
#define PCMMemory_H_

#include "/home/abasak/pcm/utils.h"
#include "/home/abasak/pcm/cpucounters.h"

//Programmable iMC counter
#define READ 0
#define WRITE 1
#define READ_RANK_A 0
#define WRITE_RANK_A 1
#define READ_RANK_B 2
#define WRITE_RANK_B 3
#define PARTIAL 2
#define PMM_READ 2
#define PMM_WRITE 3
#define NM_HIT 0  // NM :  Near Memory (DRAM cache) in Memory Mode
#define PCM_DELAY_DEFAULT 1.0 // in seconds
#define PCM_DELAY_MIN 0.015 // 15 milliseconds is practical on most modern CPUs
#define PCM_CALIBRATION_INTERVAL 50 // calibrate clock only every 50th iteration


using namespace std;

const uint32 max_sockets = 256;
const uint32 max_imc_channels = ServerUncorePowerState::maxChannels;
const uint32 max_edc_channels = ServerUncorePowerState::maxChannels;
const uint32 max_imc_controllers = ServerUncorePowerState::maxControllers;

typedef struct memdata {
    float iMC_Rd_socket_chan[max_sockets][max_imc_channels];
    float iMC_Wr_socket_chan[max_sockets][max_imc_channels];
    float iMC_PMM_Rd_socket_chan[max_sockets][max_imc_channels];
    float iMC_PMM_Wr_socket_chan[max_sockets][max_imc_channels];
    float iMC_Rd_socket[max_sockets];
    float iMC_Wr_socket[max_sockets];
    float iMC_PMM_Rd_socket[max_sockets];
    float iMC_PMM_Wr_socket[max_sockets];
    float M2M_NM_read_hit_rate[max_sockets][max_imc_controllers];
    float EDC_Rd_socket_chan[max_sockets][max_edc_channels];
    float EDC_Wr_socket_chan[max_sockets][max_edc_channels];
    float EDC_Rd_socket[max_sockets];
    float EDC_Wr_socket[max_sockets];
    uint64 partial_write[max_sockets];
    bool PMM;
} memdata_t;

/* Output format: 
socket(i) read bW, socket(i) write BW for all i; system read BW; system write BW; system total BW
*/ 
void display_bandwidth_alg(PCM *m, memdata_t *md){
    float sysReadDRAM = 0.0, sysWriteDRAM = 0.0, total = 0.0;
    
    ofstream out("AlgMem.csv", std::ios_base::app); 

    for(uint32 skt = 0; skt < m->getNumSockets(); ++skt){
        out << md->iMC_Rd_socket[skt] /* socket read BW (MB/s) */ <<  "," << md->iMC_Wr_socket[skt] /* socket write BW (MB/s) */ << ",";       
        sysReadDRAM += md->iMC_Rd_socket[skt];   // system read BW (MB/s)
        sysWriteDRAM += md->iMC_Wr_socket[skt];  // system write BW (MB/s)      
    }

    total = sysReadDRAM + sysWriteDRAM;
    out << sysReadDRAM << "," << sysWriteDRAM << "," << total << "\n";
    out.close();
}

void display_bandwidth_update(PCM *m, memdata_t *md){
    float sysReadDRAM = 0.0, sysWriteDRAM = 0.0, total = 0.0;
    
    ofstream out("UpdateMem.csv", std::ios_base::app); 
    
    for(uint32 skt = 0; skt < m->getNumSockets(); ++skt){
        out << md->iMC_Rd_socket[skt] /* socket read BW (MB/s) */ <<  "," << md->iMC_Wr_socket[skt] /* socket write BW (MB/s) */ << ",";        
        sysReadDRAM += md->iMC_Rd_socket[skt];   // system read BW (MB/s)
        sysWriteDRAM += md->iMC_Wr_socket[skt];  // system write BW (MB/s)      
    }

    total = sysReadDRAM + sysWriteDRAM;
    out << sysReadDRAM << "," << sysWriteDRAM << "," << total << "\n";
    out.close();
}

void calculate_bandwidth(PCM *m, const ServerUncorePowerState uncState1[], const ServerUncorePowerState uncState2[], uint64 elapsedTime, bool update, bool PMM)
{
    //const uint32 num_imc_channels = m->getMCChannelsPerSocket();
    //const uint32 num_edc_channels = m->getEDCChannelsPerSocket();
    memdata_t md;
    md.PMM = PMM;

    for(uint32 skt = 0; skt < m->getNumSockets(); ++skt)
    {
        md.iMC_Rd_socket[skt] = 0.0;
        md.iMC_Wr_socket[skt] = 0.0;
        md.iMC_PMM_Rd_socket[skt] = 0.0;
        md.iMC_PMM_Wr_socket[skt] = 0.0;
        md.EDC_Rd_socket[skt] = 0.0;
        md.EDC_Wr_socket[skt] = 0.0;
        md.partial_write[skt] = 0;
        for(uint32 i=0; i < max_imc_controllers; ++i)
        {
            md.M2M_NM_read_hit_rate[skt][i] = 0.;
        }
        const uint32 numChannels1 = m->getMCChannels(skt, 0); // number of channels in the first controller

	switch(m->getCPUModel()) {
	case PCM::KNL:
            for(uint32 channel = 0; channel < max_edc_channels; ++channel)
            {
                if(getEDCCounter(channel,READ,uncState1[skt],uncState2[skt]) == 0.0 && getEDCCounter(channel,WRITE,uncState1[skt],uncState2[skt]) == 0.0)
                {
                    md.EDC_Rd_socket_chan[skt][channel] = -1.0;
                    md.EDC_Wr_socket_chan[skt][channel] = -1.0;
                    continue;
                }

                md.EDC_Rd_socket_chan[skt][channel] = (float) (getEDCCounter(channel,READ,uncState1[skt],uncState2[skt]) * 64 / 1000000.0 / (elapsedTime/1000.0));
                md.EDC_Wr_socket_chan[skt][channel] = (float) (getEDCCounter(channel,WRITE,uncState1[skt],uncState2[skt]) * 64 / 1000000.0 / (elapsedTime/1000.0));

                md.EDC_Rd_socket[skt] += md.EDC_Rd_socket_chan[skt][channel];
                md.EDC_Wr_socket[skt] += md.EDC_Wr_socket_chan[skt][channel];
	    }
        default:
            for(uint32 channel = 0; channel < max_imc_channels; ++channel)
            {
                if(getMCCounter(channel,READ,uncState1[skt],uncState2[skt]) == 0.0 && getMCCounter(channel,WRITE,uncState1[skt],uncState2[skt]) == 0.0) //In case of JKT-EN, there are only three channels. Skip one and continue.
                {
                    if (!PMM || (getMCCounter(channel,PMM_READ,uncState1[skt],uncState2[skt]) == 0.0 && getMCCounter(channel,PMM_WRITE,uncState1[skt],uncState2[skt]) == 0.0))
                    {
                        md.iMC_Rd_socket_chan[skt][channel] = -1.0;
                        md.iMC_Wr_socket_chan[skt][channel] = -1.0;
                        continue;
                    }
                }

                md.iMC_Rd_socket_chan[skt][channel] = (float) (getMCCounter(channel,READ,uncState1[skt],uncState2[skt]) * 64 / 1000000.0 / (elapsedTime/1000.0));
                md.iMC_Wr_socket_chan[skt][channel] = (float) (getMCCounter(channel,WRITE,uncState1[skt],uncState2[skt]) * 64 / 1000000.0 / (elapsedTime/1000.0));

                md.iMC_Rd_socket[skt] += md.iMC_Rd_socket_chan[skt][channel];
                md.iMC_Wr_socket[skt] += md.iMC_Wr_socket_chan[skt][channel];

                if(PMM)
                {
                    md.iMC_PMM_Rd_socket_chan[skt][channel] = (float) (getMCCounter(channel,PMM_READ,uncState1[skt],uncState2[skt]) * 64 / 1000000.0 / (elapsedTime/1000.0));
                    md.iMC_PMM_Wr_socket_chan[skt][channel] = (float) (getMCCounter(channel,PMM_WRITE,uncState1[skt],uncState2[skt]) * 64 / 1000000.0 / (elapsedTime/1000.0));

                    md.iMC_PMM_Rd_socket[skt] += md.iMC_PMM_Rd_socket_chan[skt][channel];
                    md.iMC_PMM_Wr_socket[skt] += md.iMC_PMM_Wr_socket_chan[skt][channel];

                    md.M2M_NM_read_hit_rate[skt][(channel < numChannels1)?0:1] += (float)getMCCounter(channel,READ,uncState1[skt],uncState2[skt]);
                }
                else
                {
                    md.partial_write[skt] += (uint64) (getMCCounter(channel,PARTIAL,uncState1[skt],uncState2[skt]) / (elapsedTime/1000.0));
                }
            }
	}
        if (PMM)
        {
            for(uint32 c = 0; c < max_imc_controllers; ++c)
            {
                if(md.M2M_NM_read_hit_rate[skt][c] != 0.0)
                {
                    md.M2M_NM_read_hit_rate[skt][c] = ((float)getM2MCounter(c, NM_HIT, uncState1[skt],uncState2[skt]))/ md.M2M_NM_read_hit_rate[skt][c];
                }
            }
        }
    }

    if(update) display_bandwidth_update(m, &md);
    else display_bandwidth_alg(m, &md);    
}
#endif //PCMMemory_H_