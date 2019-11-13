#include <math.h>
#include <iomanip>
#include "lya.hpp"
#include "lya_tcl.hpp"
#include "db/db.hpp"
#include "core/tex/tex.hpp"
#include "lem_mac.hpp"
//#include "utils/sys/thread.hpp"

//static IntPair_t minCost( const vInt_t& vCongestCost, const vInt_t& vAdjCost, const mInt_t& mCost, int j ){}
static const int _minAux = 1;
void Lya_t::auxguide_tex_assign(int layer, const vInt_t& vTex, vIntPair_t& vPos ){
	float FillRatio = _pGdb->AuxRatio();
	int nPrivilege  = std::max( 3, (int) ((float)0.5 * FillRatio * vTex.size()) );
	int nAuxLim = (float) FillRatio * vTex.size()+1;
	nAuxLim = std::max( nAuxLim, 1+_minAux );
	if( vTex.size()<_pGdb->AuxSegSize() )
		return;
	if( nAuxLim<2 )
		return ;
	if( _pTex->layerV().size()<3 )
		return ;
	mInt_t mCost(nAuxLim);
	mInt_t mSel (nAuxLim);
	for(int i=0; i<nAuxLim; i++){
		mCost[i].resize(vTex.size(), 0);
		mSel [i].resize(vTex.size(), 0);
	}
	int adj_layer = layer==_start_layer? layer+1: layer-1;
	if( adj_layer < _start_layer || _pTex->layerV().size() <= adj_layer )
		return;
	vInt_t vCongestCost( vTex.size(), 0 );
	vInt_t vAdjCost    ( vTex.size(), 0 );
	for(int j=0; j<vTex.size(); j++){

		int obs     = get_obsLoad (vTex[j],     layer);
		int via     = get_viaLoad (vTex[j],     layer);
		int wire    = get_wireLoad(vTex[j],     layer);
		int adj_obs = get_obsLoad (vTex[j], adj_layer);
		int adj_via = get_viaLoad (vTex[j], adj_layer);
		int adj_wire= get_wireLoad(vTex[j], adj_layer);
		vCongestCost[j] =      via+     wire+     obs;
		vAdjCost    [j] =  adj_via+ adj_wire+ adj_obs;
	}


	for( int j=0; j<nAuxLim; j++ )
		for(int z=0; z<vTex.size(); z++){
			if( 0==j ){
				mCost[j][z] = (0<z? mCost[j][z-1]: 0) + vCongestCost[z];
				mSel [j][z] = 0;
				continue;
			}
			// search min insertion of previous aux
			vInt_t vAccuCost(z+1,0);
			for(int m=0; m<z; m++){
				vAccuCost[m] = mCost[j-1][m];
				for(int n=m+nPrivilege; n<=z; n++)
					vAccuCost[m] += vCongestCost[n];
			}
			int min_prev_z = 0;
			for(int m=0; m<z; m++)
				if( vAccuCost[m] < vAccuCost[min_prev_z] )
					min_prev_z = m;

			mCost[j][z]= vAdjCost[z] + vAccuCost[min_prev_z];// + ((z<min_prev_z+nPrivilege)? 0: vCongestCost[z]);
			mSel [j][z]= min_prev_z;
		}
	
	for(int z=0; z<vTex.size(); z++)
		for(int m=z+nPrivilege; m<vTex.size(); m++)
			mCost[nAuxLim-1][z] += vCongestCost[m];
	

	int min_end = 0;
	for(int z=0; z<vTex.size(); z++){
		if( mCost[nAuxLim-1][z] < mCost[nAuxLim-1][min_end] )
			min_end = z;
	}
	int jcur= nAuxLim-1;
	int sel = mSel[jcur][min_end];
	vPos.clear();
	while( jcur > 0 ){
		vPos.push_back(IntPair_t(adj_layer, vTex[sel]));
		if( 0==sel )
			break;
		sel = mSel[--jcur][sel];
	}
	//std::cout<< vTex.size()<<std::endl;
//	if( vTex.size()==23 ){
//		std::cout<<"min_end="<< min_end<<std::endl;
//
//		for(int z=0; z<vTex.size(); z++)
//			std::cout<<std::setw(5)<<z<<" ";
//		std::cout<<std::endl;
//		std::cout<<"vAdjCost:"<<std::endl;
//		for(int z=0; z<vTex.size(); z++)
//			std::cout<<std::setw(5)<<vAdjCost[z]<<" ";
//		std::cout<<std::endl;
//		std::cout<<"vCongestCost:"<<std::endl;
//		for(int z=0; z<vTex.size(); z++)
//			std::cout<<std::setw(5)<<vCongestCost[z]<<" ";
//		std::cout<<std::endl;
//		std::cout<<"mCost="<<std::endl;
//		for( int j=0; j<nAuxLim; j++ ){
//			for(int z=0; z<vTex.size(); z++)
//				std::cout<<std::setw(5)<<mCost[j][z]<<" ";
//			std::cout<<std::endl;
//		}
//		std::cout<<std::endl;
//
//		for(int z=0; z<vTex.size(); z++)
//			std::cout<<std::setw(3)<<z<<" ";
//		std::cout<<"\n";
//		for( int j=0; j<nAuxLim; j++ ){
//			for(int z=0; z<vTex.size(); z++)
//				std::cout<<std::setw(3)<<mSel [j][z]<<" ";
//			std::cout<<std::endl;
//		}
//	}
}

void Lya_t::auxguide_seg_array(vSegPtr_t& vSegPtr, bool isV){
	for(int i=0; i<vSegPtr.size(); i++){
		Seg_t& Seg = *vSegPtr[i];
		Seg.vAux().clear();
		for(int j=0; j<Seg.size(); j++){
			int k=j+1;
			for(; k<Seg.size(); k++)
				if( Seg.Layer(k)!=Seg.Layer(j) )
					break;
			if( Seg.size()<=k || k-j<_pGdb->AuxSegSize() ){
				j=k-1;
				continue;
			}
			int layer = Seg.Layer(j);
			vInt_t vTex;
			for(int m=j; m<k; m++)
				vTex.push_back(Seg[m]);
			vIntPair_t vRes;
			auxguide_tex_assign(layer, vTex, vRes);
			Seg.vAux().insert(Seg.vAux().end(), vRes.begin(), vRes.end());
			j=k-1;
		}
	}
}
