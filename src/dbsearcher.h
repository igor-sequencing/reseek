#pragma once

#include "profileloader.h"
#include "dbsearcher.h"
#include "dssaligner.h"
#include "xdpmem.h"
#include <atomic>
#include <map>
#include <mutex>

class ChainReader2;

class DBSearcher
	{
public:
	~DBSearcher();

public:
	mutex m_Lock;
	const DSSParams *m_Params = 0;
	uint m_ThreadCount = UINT_MAX;
	vector<DSSAligner *> m_DAs;
	vector<XDPMem *> m_Mems;

	vector<PDBChain *> m_DBChains;
	bool m_QuerySelf = false;

// Per-chain vectors [ChainIdx]
	vector<vector<vector<byte> > *> m_DBProfiles;
	vector<vector<byte> *> m_DBMuLettersVec;
	vector<vector<uint> *> m_DBMuKmersVec;
	vector<float> m_DBSelfRevScores;

	uint m_PairIndex = UINT_MAX;
	uint m_PairCount = UINT_MAX;
	uint m_NextChainIndex1 = UINT_MAX;
	uint m_NextChainIndex2 = UINT_MAX;
	uint m_NextQueryIdx = UINT_MAX;
	uint m_NextDBIdx = UINT_MAX;

	atomic<uint> m_ProcessedQueryCount = 0;
	atomic<uint> m_ProcessedPairCount = 0;
	atomic<uint> m_HitCount = 0;
	atomic<uint> m_QPCacheHits = 0;
	atomic<uint> m_QPCacheMisses = 0;

	uint m_FilterRejects = 0;
	uint m_XAlignCount = 0;
	uint m_SWAlignCount = 0;
	uint m_UFilterCount = 0;
	float m_MaxEvalue = 10;
	uint m_Secs = UINT_MAX;
	float m_AlnsPerThreadPerSec = FLT_MAX;
	time_t m_LastProgress = 0;

public:
	void Setup();
	void InitEmpty();
	void LoadDB(const string &DBFN);
	uint GetDBChainCount() const { return SIZE(m_DBChains); }

	void RunQuery(ChainReader2 &QCR);
	void RunSelf();

	void ThreadBodyQuery(uint ThreadIndex, ChainReader2 *ptrQueryCR);
	void ThreadBodySelf(uint ThreadIndex);

	uint GetDBSize() const;
	bool GetNextPairSelf(uint &ChainIndex1, uint &ChainIndex2);
	void RunStats() const;
	void AddChain(PDBChain *ptrChain, vector<vector<byte> > *ptrProfile,
	  vector<byte> *ptrMuLetters);

public:
	virtual void OnSetup() {}
	void BaseOnAln(DSSAligner &DA, bool Up);
	virtual void OnAln(DSSAligner &DA, bool Up) {}

public:
	static void StaticThreadBodyQuery(uint ThreadIndex, DBSearcher *ptrDBS, ChainReader2 *ptrQueryCR);
	static void StaticThreadBodySelf(uint ThreadIndex, DBSearcher *ptrDBS);
	};
