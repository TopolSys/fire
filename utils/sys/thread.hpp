#ifndef UTILS_SYS_THREAD_HPP
#define UTILS_SYS_THREAD_HPP

#include <set>
#include <list>
#include <string>
#include <iostream>

#include <map>

#include <atomic>
#include <thread>

#include "lem_mac.hpp"
namespace lem {

class Spin_t {
private:
	typedef enum {Locked, Unlocked} LockState;
	std::atomic<LockState> state_;
public:
	Spin_t() : state_(Unlocked) {}
	bool try_lock(){
		return state_.exchange(Locked, std::memory_order_acquire) != Locked;
	}
	void lock(){
		while (state_.exchange(Locked, std::memory_order_acquire) == Locked);
	}
	void unlock(){
		state_.store(Unlocked, std::memory_order_release);
	}
};
class Deptor_t {
private:
	bool _fisFree;
public:
	Deptor_t(bool fisFree=false):_fisFree(fisFree){}
	bool isFree() const { return _fisFree; }
	bool operator()(const int& n1, const int& n2) const {return false;}
};
template<typename DEPTOR_T=Deptor_t >
class Jidep_t: public Spin_t { // Job-id Dependency 
public:
	static const int jobWait() { return -1; }
	static const int jobNull() { return -2; }
	Jidep_t( int buff_size = 100 ){
		_para_init(buff_size);
	}
	Jidep_t( const vInt_t& vJid, const DEPTOR_T& Deptor, int buff_size = 100 ){
		_para_init(buff_size);
		init(vJid,Deptor);
	}
private:
	// initialize 
	DEPTOR_T _Deptor;
	vInt_t _vJid, _vJid2Index;
	vBool_t _vDone;
	int _buff_size;
	int _nDep, _nWaitLv2; // statictic
	void _para_init( int buff_size ){
		_buff_size = buff_size;
		_topAll = 0;
		_topBuf = 0;
		_endBuf = 0;
		_nDep   = 0;
		_nWaitLv2 = 0;
		_hasLegal.store(false);
		_nDone.store(0);
		_nFreeJobTop.store(0);
	}
	void _resolve( int from, int to ){
		// Watch, Child
		spinRel.lock();
		_Child.clear();
		_Watch.clear();
		if( lem::dev() )
			assert( _LegalJob.empty() );
		for(int i=from; i<to; i++)
			for(int j=i+1; j<to; j++){
				if( ! _Deptor(_vJid[i],_vJid[j]) )
					continue;
				_nDep++;
				_Child.insert( JobDep_t::value_type(i,j) ); // i has child j
				_Watch[j] ++;
			}

		// Watch, LegalJob
		for(int i=from; i<to ; i++){
			if( _Watch.find(i) != _Watch.end() )
				continue;
			LegalJobPush(i); //_LegalJob.insert(i);
		}
		spinRel.unlock();
		// LegalJob
		_check_hasLegal();
	}
	//bool _buffer_finish() const {\
		return _nDone.load() >= _endBuf;\
	}
	void _check_buffer_update(){
		// Buffer
		if( _Deptor.isFree() )
			return ;
		spinBuf.lock();
		if( _nDone.load() < _endBuf )
			goto END;
		_topBuf = _endBuf;
		_endBuf = _topBuf + _buff_size;
		if( _endBuf > _vJid.size() )
			_endBuf = _vJid.size();
		spinBuf.unlock();
		
		_resolve( _topBuf, _endBuf );
		return;
	END:
		spinBuf.unlock();
	}

	// frequently change
	int _topAll, _topBuf, _endBuf;
	std::atomic<int> _nDone, _nFreeJobTop;
	typedef std::multimap<int,int,std::less<int> > JobDep_t;
	typedef std::pair<JobDep_t::iterator,JobDep_t::iterator> JobDepRange_t;
	JobDep_t _Child;
	std::map<int,int> _Watch;
	std::set<int> _LegalJob;
	std::atomic<bool> _hasLegal;
	Spin_t spinBuf, spinRel, spinLegal, spinFree;
	int LegalJobPop(){
		int ret = jobWait();
		spinLegal.lock();
		if( _LegalJob.empty() )
			goto END;
		ret = *_LegalJob.begin();
		_LegalJob.erase(_LegalJob.begin());
		
	END:
		_check_hasLegal();
		spinLegal.unlock();
		return ret;
	}
	void LegalJobPush(int index){
		spinLegal.lock();
		_LegalJob.insert(index);
		_check_hasLegal();
		spinLegal.unlock();
	}
	void _check_hasLegal(){
		if( _LegalJob.empty() )
			_hasLegal.store(false);
		else
			_hasLegal.store(true );
	}
	int _getJob(){
		if( done() )
			return jobNull();

		if( _Deptor.isFree() ){ // no dependency requirement 
			spinFree. lock();
			int ret = _nFreeJobTop.load();
			if( ret>=_vJid.size() ){
				spinFree.unlock();
				return jobNull();
			}
			_nFreeJobTop.fetch_add(1);
			spinFree.unlock();
			//if(ret>=_vJid.size())\
				std::cout<<"!!!!! "<<ret<<","<<_vJid.size()<<std::endl;\
			assert(ret<_vJid.size());
			return _vJid[ret];
		}

		//if( _buffer_finish() )\
			_update_buffer();
		//_check_buffer_update();

		int index = LegalJobPop();
		if(jobWait() != index)
			return _vJid[index];
//		if( ! _LegalJob.empty() ){
//			int ret = *_LegalJob.begin();
//			_LegalJob.erase(_LegalJob.begin());
//			_check_hasLegal();
//			return _vJid[ret];
//		}
		_nWaitLv2++;
		return jobWait();
	}
public:
	int nDep() const { return _nDep; }
	int nWaitLv2() const { return _nWaitLv2; }
	int nDone() const { return _nDone.load(); }

	// cold section
	void init( const vInt_t& vJid, const DEPTOR_T& Deptor ){
		lock();
		_nDone.store(0);
		_hasLegal.store(false);
		_Deptor = Deptor;
		_vJid = vJid;
		
		_topBuf = 0;
		_endBuf = 0;

		_vDone.clear();
		_vDone.resize( _vJid.size(), false );
		_LegalJob.clear();
		_check_hasLegal();
		_Watch.clear();
		_Child.clear();
		
		int maxJid = 0;
		for(int i=0; i<_vJid.size(); i++)
			maxJid = std::max(maxJid, _vJid[i]);
		_vJid2Index.clear();
		_vJid2Index.resize( maxJid+1, -1);
		for(int i=0; i<_vJid.size(); i++)
			_vJid2Index[ _vJid[i] ] = i;

		if( !_Deptor.isFree() )
			_check_buffer_update(); //_update_buffer();
		unlock();
	}
	// atomic
	bool done() const { return _vJid.size()==_nDone.load(); }
	void done( int jid ){
		JobDepRange_t Child;
		int index;
		if( _Deptor.isFree() )
			goto END;
		
		spinRel.lock();
		index = _vJid2Index[jid];
		_vDone[index] = true;
		Child = _Child.equal_range(index);
		for( JobDep_t::iterator itr = Child.first; itr != Child.second; itr ++ ){
			int cid = itr->second;
			std::map<int,int>::iterator citr = _Watch.find(cid);
			if( lem::dev() ){
				assert( _Watch.end() != citr );
				assert( 1<=citr->second );
			}
			citr->second --; // decrease load
			if( 0==citr->second ){
				//_LegalJob.insert(cid);
				LegalJobPush(cid);
				_Watch.erase(citr);
			}
		}
		spinRel.unlock();

		_check_hasLegal();
		//if( _buffer_finish() )\
			_update_buffer();
		END:
		_nDone.fetch_add(1);
		if( !_Deptor.isFree() )
			_check_buffer_update();
	}
	void print_watch(){
		for( std::map<int,int>::iterator itr=_Watch.begin(); itr!=_Watch.end(); itr++)
			std::cout<< itr->first <<" has "<< itr->second <<" watch left\n";
	}

	// atomic
	bool preserveJob(){ return _hasLegal.exchange(false); }
	
	int getJob(){
		if( done() )
			return jobNull();
		if( _Deptor.isFree()? false: ! preserveJob() )\
			return jobWait();
		// busy section
		//lock();
		int ret = _getJob();
		//unlock();
		return ret;
	}
};

};

#endif