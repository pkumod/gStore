#include "PathQueryHandler.h"

using namespace std;

/**
 * @brief Supports LDBC SNB IC14 (v0.x and v1.x, old).
 * Assumes both uid and vid are of the type Person.
 * 
 * @param uid Person1
 * @param vid Person2
 * @return a vector of all paths and their weights as per IC14's requirements
 */
vector<pair<vector<int>, double>> PathQueryHandler::IC14(int uid, int vid, int knowsPred, int hasCreatorPred,
int typePred, int replyPred, int postId, int commentId) {
	vector<pair<vector<int>, double>> ret;
	if (uid == vid)
		return ret;
	vector<int> pred_set(1, knowsPred);
	vector<int> sp = shortestPath(uid, vid, true, pred_set);
	if (sp.empty())
		return ret;
	size_t spSz = sp.size();
	size_t spLen = (spSz - 1) / 2;
	vector<vector<int>> allSp = kHopEnumeratePath(uid, vid, -1, true, spLen, pred_set);

	// Collate all vertex pairs appearing in any shortest path
	unordered_map<int, unordered_map<int, double>> edgeWeight;
	for (const auto &oneSp : allSp) {
		for (size_t i = 0; i < spSz - 2; i += 2)
			edgeWeight[oneSp[i]][oneSp[i + 2]] = 0;
	}
	
	// Calculate weights
	vector<int> leftPost, leftComm, rightPost, rightComm;
	unordered_set<int> leftCommReplyOf, rightCommReplyOf;
	for (auto &e : edgeWeight) {
		getPostComments(e.first, hasCreatorPred, typePred, postId, commentId, leftPost, leftComm);
		getCommReplyOf(replyPred, leftComm, leftCommReplyOf);
		for (auto &dstW : e.second) {
			getPostComments(dstW.first, hasCreatorPred, typePred, postId, commentId, rightPost, rightComm);
			getCommReplyOf(replyPred, rightComm, rightCommReplyOf);
			
			size_t comm2post = 0, comm2comm = 0;
			for (int rp : rightPost) {
				if (leftCommReplyOf.find(rp) != leftCommReplyOf.end())
					comm2post++;
			}
			for (int rc : rightComm) {
				if (leftCommReplyOf.find(rc) != leftCommReplyOf.end())
					comm2comm++;
			}
			for (int lp : leftPost) {
				if (rightCommReplyOf.find(lp) != rightCommReplyOf.end())
					comm2post++;
			}
			for (int lc : leftComm) {
				if (leftCommReplyOf.find(lc) != leftCommReplyOf.end())
					comm2comm++;
			}
			dstW.second = double(comm2post) + double(comm2comm) * .5;
		}
	}

	// Report weights
	for (auto &oneSp : allSp) {
		double totalWeight = 0;
		for (size_t i = 0; i < spSz - 2; i += 2)
			totalWeight += edgeWeight[oneSp[i]][oneSp[i + 2]];
		ret.emplace_back(std::move(oneSp), totalWeight);
	}
	return ret;
}