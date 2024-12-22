#pragma once

#include "profileloader.h"
#include "dbsearcher.h"
#include "dssaligner.h"
#include "xdpmem.h"
#include <atomic>
#include <map>
#include <mutex>
#include "mukmerfilter.h"

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
	FILE *m_fTsv = 0;
	FILE *m_fAln = 0;
	FILE *m_fFasta2 = 0;
	uint m_Secs = UINT_MAX;
	float m_AlnsPerThreadPerSec = FLT_MAX;
	time_t m_LastProgress = 0;

#if SLOPE_CALIB
// Calibrated slopes
//	PredMinusLogP = m*TS + b;
// ms & bs vectors indexed by ChainIdx
	vector<float> m_ms;
	vector<float> m_bs;
#endif

#if GUMBEL_CALIB
	vector<float> m_Gumbel_mus;
	vector<float> m_Gumbel_betas;
#endif

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
	float GetSelfRevScore(const PDBChain &Chain,
	  const vector<vector<byte> > &Profile, DSSAligner &DA);

#if SLOPE_CALIB
// Slope calibrated runtime
	void LoadCalibratedSlopes(const string &FN);
	void GetChainSlope(uint ChainIdx, float &m, float &b) const;
#endif

#if GUMBEL_CALIB
	void LoadGumbelCalib(const string &FN);
	void GetChainGumbel(uint ChainIdx, float &mu, float &beta) const;
#endif

public:
	virtual void OnSetup() {}
	void BaseOnAln(DSSAligner &DA, bool Up);
	virtual void OnAln(DSSAligner &DA, bool Up) {}

public:
	static void StaticThreadBodyQuery(uint ThreadIndex, DBSearcher *ptrDBS, ChainReader2 *ptrQueryCR);
	static void StaticThreadBodySelf(uint ThreadIndex, DBSearcher *ptrDBS);
	};
