/*=============================================================================
# Filename: TPSelectivity.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2018-08-04
# Description:
=============================================================================*/

#ifndef _TP_SELECTIVITY_H
#define _TP_SELECTIVITY_H

#include "../Util/Util.h"
#include "../KVstore/KVstore.h"

class TPSelectivity
{
	private:
		struct PairHash
		{
    		std::size_t operator()(const std::pair<unsigned, unsigned> &p) const noexcept
    		{
        		std::size_t h_first = std::hash<unsigned>{}(p.first);
        		std::size_t h_second = std::hash<unsigned>{}(p.second);
        		return h_first ^ h_second; 
    		}
		};

		KVstore *kvstore_;
		std::unordered_map<std::pair<unsigned, unsigned>, unsigned, PairHash> sp2o_{}, op2s_{};
		std::unordered_map<unsigned, unsigned> p2so_{};

	public:
		explicit TPSelectivity(KVstore *kvstore) :kvstore_(kvstore) {}

		inline unsigned getSP2O(unsigned sid, unsigned pid)
		{
			std::pair<unsigned, unsigned> key(sid, pid);

			if (this->sp2o_.count(key) == 0)
				this->sp2o_[key] = this->kvstore_->getSubjectPredicateDegree(sid, pid);

			return this->sp2o_[key];
		}

		inline unsigned getOP2S(unsigned oid, unsigned pid)
		{
			std::pair<unsigned, unsigned> key(oid, pid);

			if (this->op2s_.count(key) == 0)
				this->op2s_[key] = this->kvstore_->getObjectPredicateDegree(oid, pid);

			return this->op2s_[key];
		}

		inline unsigned getP2SO(unsigned pid)
		{
			if (this->p2so_.count(pid) == 0)
				this->p2so_[pid] = this->kvstore_->getPredicateDegree(pid);

			return this->p2so_[pid];
		}
};

#endif // _TP_SELECTIVITY_H
