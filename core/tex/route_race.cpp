#include <map>       // point to index in terminal parition
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "utils/sys/sys.hpp" // error log

#include "db/db.hpp"
#include "core/tex/tex.hpp"
#include "route.hpp"
#include "round.hpp"

#include "lem_mac.hpp"
#include "lem_type.hpp"

#include "alg/rsmt/rsmt.hpp"

static void SteinerTree( Lem_Gdb_t * pGdb ){
	int Lem_NetSteinerPoint( Tcl_t& Tcl, Rsmt_t& rsmt );
	for(int i=0; i<pGdb->vTcl.size(); i++){
		Tcl_t& Tcl=pGdb->vTcl[i];
		if( ! Lem_NetSteinerPoint( Tcl, Tcl.Rsmt() ) ){
			char buff[1024];
			sprintf(buff, "net_id = %d steiner tree failure",i);
			lem::error.add(buff);
			Tcl.notOk();
			continue;
		}
	}
}

static lem::Spin_t progSpin;
static void InitRaceTcl( Lem_Gdb_t * pGdb, Tex_Man_t * pTex, lem::Jidep_t<>* pJidep, lem::Progress_t<int>* pProg ){
	void Lem_Tcl2TexPos( Tcl_t& Tcl );
	int Lem_NetDecomp( Tcl_t& Tcl, vIntPair_t& vPinPair );

	lem::Jidep_t<>& Jidep = *pJidep;
	int jid = lem::Jidep_t<>::jobWait();
	while( lem::Jidep_t<>::jobNull() != (jid=Jidep.getJob()) ){
		Tcl_t& Tcl = pGdb->vTcl[jid];
		Lem_Tcl2TexPos( Tcl );
		Lem_NetDecomp( Tcl,Tcl.vTegPair() );
		if( Tcl.mTegPath().size() < Tcl.vTegPair().size() ){
			Tcl.mTegPath().clear();
			Tcl.mTegPath().resize( Tcl.vTegPair().size() );
		}
		if( Tcl.mTegTeid().size() < Tcl.vTegPair().size() ){
			Tcl.mTegTeid().clear();
			Tcl.mTegTeid().resize( Tcl.vTegPair().size() );
		}
		Jidep.done(jid);

		progSpin.lock();
		if( 0==(Jidep.nDone()%100) || jid==(pGdb->vTcl.size()-1))
			pProg->set(Jidep.nDone()).print(std::cout);
		progSpin.unlock();
	}
	
	progSpin.lock();
	if( Jidep.nDone() )
		pProg->set(Jidep.nDone()).print(std::cout);
	progSpin.unlock();
}

static void InitRace( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	std::cout<<"Init"<<std::endl;
	vInt_t vJid(pGdb->vTcl.size());
	for(int i=0; i<vJid.size(); i++)
		vJid[i] = i;
	lem::Jidep_t<> Jidep(vJid, lem::Deptor_t(true));

	const int nChildThread = pGdb->nThread()-1;
	std::vector<std::thread> vChildThread(nChildThread);
	lem::Progress_t<int> prog( 50, pGdb->vTcl.size());
	for(int i=0; i<nChildThread; i++)
		vChildThread[i] = std::thread( &InitRaceTcl, pGdb, pTex, &Jidep, &prog );
	InitRaceTcl(pGdb,pTex,&Jidep,&prog);
	for(int i=0; i<nChildThread; i++)
		vChildThread[i].join();
	std::cout<<std::endl;
}

static void TrialRouteRaceTcl( Lem_Gdb_t * pGdb, Tex_Man_t * pTex, lem::Jidep_t<>* pJidep, lem::Progress_t<int>* pProg ){
	Route_t::argpk_t argpk(pGdb,pTex);
	argpk._StepBase   = 200;
	argpk._StepOffset = 5;
	argpk._PredOffset = 0;
	argpk._CostOffset = 0;
	argpk._fUseStepLimit = false;
	Route_t route(argpk);

	lem::Progress_t<int> prog( 50, pGdb->vTcl.size());
	
	lem::Jidep_t<>& Jidep = *pJidep;
	int jid = lem::Jidep_t<>::jobWait();
	while( lem::Jidep_t<>::jobNull() != (jid=Jidep.getJob()) ){
		Tcl_t& Tcl = pGdb->vTcl[jid];
		route.minlen_obs_cross(Tcl);
		Jidep.done(jid);
		progSpin.lock();
		if( 0==(Jidep.nDone()%100) || jid==(pGdb->vTcl.size()-1))
			pProg->set(Jidep.nDone()).print(std::cout);
		progSpin.unlock();
	}
	progSpin.lock();
	if( Jidep.nDone() )
		pProg->set(Jidep.nDone()).print(std::cout);
	progSpin.unlock();
}


static int Add_initial_demand( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	int nNet = 0;
	vInt_t vNetID;
	Lem_IterateCount(pGdb->vTcl,pTcl,nNet){
		if(!pTcl->vPath().empty())
			continue;
		vNetID.push_back(nNet);
	}
	if( !vNetID.empty() ){
		for(int i=0; i<vNetID.size() && i<5; i++){
			int tcl = vNetID[i];
			char buff[2048];
			snprintf(buff,2047,"Net \'%s\' (id=%d) is empty after initial route. (%d/%d)", pGdb->vTcl[tcl].name().c_str(), tcl, i+1, vNetID.size() );
			lem::warning.add(buff);
		}
	}
	Route_t demand_helper( pGdb, pTex );
	Lem_Iterate(pGdb->vTcl,pTcl)
		demand_helper.demand_add_extern(pTcl->vTeid());
	if( lem::dev() )
		std::cout<<"Initial routing result are loaded"<<std::endl;
	return nNet - vNetID.size();
}

static int CleanUp_trial_route( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	int nNet = 0;
	Lem_IterateCount(pGdb->vTcl,pTcl,nNet){
		pTcl->vPath().clear();
		pTcl->vTeid().clear();
	}
	if( lem::dev() )
		std::cout<<nNet<<" initial routing results are clean up"<<std::endl;
	return nNet;
}

static void TrialRouteRace( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	std::cout<<"TrialRoute"<<std::endl;
	vInt_t vJid(pGdb->vTcl.size());
	for(int i=0; i<vJid.size(); i++)
		vJid[i] = i;
	lem::Jidep_t<> Jidep(vJid, lem::Deptor_t(true));

	const int nChildThread = pGdb->nThread()-1;
	std::vector<std::thread> vChildThread(nChildThread);
	lem::Progress_t<int> prog( 50, pGdb->vTcl.size());
	for(int i=0; i<nChildThread; i++)
		vChildThread[i] = std::thread( &TrialRouteRaceTcl, pGdb, pTex, &Jidep, &prog );
	TrialRouteRaceTcl(pGdb,pTex,&Jidep,&prog);
	for(int i=0; i<nChildThread; i++)
		vChildThread[i].join();
	std::cout<<std::endl; // end of progress bar 

	int nLarger = 0;
	for(int i=0; i<pGdb->vTcl.size(); i++){
		const Tcl_t& Tcl = pGdb->vTcl[i];
		for(int j=0; j<Tcl.TegBaseLen().size(); j++){
			Tex_Man_t::Pos_t pos1 = Tcl.vTexPos()[Tcl.vTegPair()[j].first ];
			Tex_Man_t::Pos_t pos2 = Tcl.vTexPos()[Tcl.vTegPair()[j].second];
			int opt_dist = manh(pos1, pos2);
			if( opt_dist<Tcl.TegBaseLen(j) )
				nLarger ++ ;
		}
	}
	if( pGdb->ReuseTrial() )
		Add_initial_demand(pGdb, pTex);
	else
		CleanUp_trial_route(pGdb, pTex);
	if( lem::dev() )
		std::cout<<" # 2-teg pair Larger than Manh = "<<nLarger<<std::endl;
}

void Lem_InitRouteRace( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	InitRace(pGdb,pTex);
	SteinerTree(pGdb);
	TrialRouteRace(pGdb,pTex);
}