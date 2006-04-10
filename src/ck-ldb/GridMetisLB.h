#ifndef _GRIDMETISLB_H_
#define _GRIDMETISLB_H_

#include <limits.h>
#include <stdio.h>

#include "charm++.h"
#include "cklists.h"

#include "CentralLB.h"

extern "C" void METIS_PartGraphRecursive (int*, int*, int*, int*, int*, int*,
					  int*, int*, int*, int*, int*);

extern "C" void METIS_PartGraphKway (int*, int*, int*, int*, int*, int*,
				     int*, int*, int*, int*, int*);

extern "C" void METIS_PartGraphVKway (int*, int*, int*, int*, int*, int*,
				      int*, int*, int*, int*, int*);

extern "C" void METIS_WPartGraphRecursive (int*, int*, int*, int*,
					   int*, int*, int*, int*,
					   float*, int*, int*, int*);

extern "C" void METIS_WPartGraphKway (int*, int*, int*, int*,
				      int*, int*, int*, int*,
				      float*, int*, int*, int*);

extern "C" void METIS_mCPartGraphRecursive (int*, int*, int*, int*,
					    int*, int*, int*, int*,
					    int*, int*, int*, int*);

extern "C" void METIS_mCPartGraphKway (int*, int*, int*, int*, int*,
				       int*, int*, int*, int*, int*,
				       int*, int*, int*);

#if CONVERSE_VERSION_VMI
extern "C" int CmiGetCluster (int process);
#endif

void CreateGridMetisLB ();

class PE_Data_T
{
  public:
    CmiBool available;
    int cluster;
    int num_objs;
    int num_lan_objs;
    int num_lan_msgs;
    int num_wan_objs;
    int num_wan_msgs;
    double relative_speed;
    double scaled_load;
};

class Object_Data_T
{
  public:
    CmiBool migratable;
    int cluster;
    int from_pe;
    int to_pe;
    int num_lan_msgs;
    int num_wan_msgs;
    double load;
};

class GridMetisLB : public CentralLB
{
  public:
    GridMetisLB (const CkLBOptions &);
    GridMetisLB (CkMigrateMessage *msg);

    void work (CentralLB::LDStats *stats, int count);

    void pup (PUP::er &p) { CentralLB::pup (p); }

  private:
    int Get_Cluster (int pe);
    int Find_Maximum_WAN_Object (int cluster);
    int Find_Minimum_WAN_PE (int cluster);
    void Assign_Object_To_PE (int target_object, int target_pe);
    CmiBool QueryBalanceNow (int step);

    int Num_PEs;
    int Num_Objects;
    int Num_Clusters;
    PE_Data_T *PE_Data;
    Object_Data_T *Object_Data;
};

#endif
