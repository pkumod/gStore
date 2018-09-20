/*=============================================================================
# Filename: TPSelectivity.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Description:
=============================================================================*/

#ifndef _TP_SELECTIVITY_H
#define _TP_SELECTIVITY_H

#include "../Util/Util.h"
#include "../KVstore/KVstore.h"
#include "QueryTree.h"

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

		unsigned getSP2O(unsigned sid, unsigned pid);
		unsigned getOP2S(unsigned oid, unsigned pid);
		unsigned getP2SO(unsigned pid);
		unsigned getTPSelectivity(const QueryTree::GroupPattern::Pattern &pattern);
};

#endif // _TP_SELECTIVITY_H
