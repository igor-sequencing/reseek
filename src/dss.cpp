#include "myutils.h"
#include "pdbchain.h"
#include "abcxyz.h"
#include "alpha.h"
#include "dss.h"

uint GetPatternOnes(const string &Str)
	{
	uint n = 0;
	for (uint i = 0; i < SIZE(Str); ++i)
		if (Str[i] == '1')
			++n;
	return n;
	}

void DSS::SetSS()
	{
	if (m_SS.empty())
		m_Chain->GetSS(m_SS);
	}

uint DSS::Get_SS(uint Pos)
	{
	SetSS();
	char c = m_SS[Pos];
	uint Letter = SSCharToInt(c);
	return Letter;
	}

uint DSS::Get_NENSS3(uint Pos)
	{
	SetSS();
	uint NEN = GetNEN(Pos);
	if (NEN == UINT_MAX)
		return WILDCARD;
	char c = m_SS[NEN];
	switch (c)
		{
	case 'h': return 0;
	case 's': return 1;
	case 't': return 2;
	case '~': return 2;
		}
	return WILDCARD;
	}

uint DSS::Get_RENSS3(uint Pos)
	{
	SetSS();
	uint NEN = GetREN(Pos);
	if (NEN == UINT_MAX)
		return WILDCARD;
	char c = m_SS[NEN];
	switch (c)
		{
	case 'h': return 0;
	case 's': return 1;
	case 't': return 2;
	case '~': return 2;
		}
	return WILDCARD;
	}

uint DSS::Get_SS3(uint Pos)
	{
	SetSS();
	char c = m_SS[Pos];
	switch (c)
		{
	case 'h': return 0;
	case 's': return 1;
	case 't': return 2;
	case '~': return 2;
		}
	return WILDCARD;
	}

void DSS::GetSSEs(uint MinLength, vector<uint> &Los,
  vector<uint> &Lengths, vector<char> &cs) const
	{
	Los.clear();
	Lengths.clear();
	cs.clear();
	const uint L = GetSeqLength();
	asserta(SIZE(m_SS) == L);
	char currc = m_SS[0];
	uint StartPos = 0;
	uint RunLength = 1;
	for (uint Pos = 1; Pos <= L; ++Pos)
		{
		char ss = (Pos == L ? 0 : m_SS[Pos]);
		if (m_SS[Pos] == currc)
			++RunLength;
		else
			{
			if (RunLength >= MinLength)
				{
				if (currc == 'h' || currc == 's')
					{
					Los.push_back(StartPos);
					Lengths.push_back(RunLength);
					cs.push_back(currc);
					}
				}
			currc = m_SS[Pos];
			StartPos = Pos;
			RunLength = 1;
			}
		}
	}

uint DSS::SSCharToInt(char c)
	{
	switch (c)
		{
	case 'h': return 0;
	case 's': return 1;
	case 't': return 2;
	case '~': return 3;
		}
	asserta(false);
	return UINT_MAX;
	}

uint DSS::SSCharToInt3(char c)
	{
	switch (c)
		{
	case 'h': return 0;
	case 's': return 1;
	case 't': return 2;
	case '~': return 2;
		}
	asserta(false);
	return UINT_MAX;
	}

void DSS::SetSSEs()
	{
	if (!m_SSE_Mids.empty())
		return;
	asserta(m_SSE_cs.empty());
	SetSS();
	vector<uint> Los;
	vector<uint> Lengths;
	GetSSEs(m_SSE_MinLength, Los, Lengths, m_SSE_cs);
	const uint N = SIZE(Los);
	asserta(SIZE(Lengths) == N);
	asserta(SIZE(m_SSE_cs) == N);
	for (uint i = 0; i < N; ++i)
		{
		uint Mid = Los[i] + Lengths[i]/2;
		m_SSE_Mids.push_back(Mid);
		}
	}

double DSS::GetFloat_NormDens(uint Pos)
	{
	SetDensity_ScaledValues();
	asserta(Pos < SIZE(m_Density_ScaledValues));
	return m_Density_ScaledValues[Pos];
	}

double DSS::GetFloat_HelixDens(uint Pos)
	{
	return GetSSDensity(Pos, 'h');
	}

double DSS::GetFloat_StrandDens(uint Pos)
	{
	return GetSSDensity(Pos, 's');
	}

//double DSS::GetFloat_LoopDens(uint Pos)
//	{
//	return GetSSDensity(Pos, '~');
//	}

void DSS::SetDensity_ScaledValues()
	{
	if (!m_Density_ScaledValues.empty())
		return;
	const uint L = GetSeqLength();
	vector<double> Values;
	m_Density_ScaledValues.reserve(L);
	Values.reserve(L);
	double MinValue = 999;
	double MaxValue = 0;
	for (uint Pos = 0; Pos < L; ++Pos)
		{
		double D = GetDensity(Pos);
		Values.push_back(D);
		if (D != DBL_MAX)
			{
			MinValue = min(MinValue, D);
			MaxValue = max(MaxValue, D);
			}
		}

	double Range = (MaxValue - MinValue);
	if (Range < 1)
		Range = 1;
	for (uint Pos = 0; Pos < L; ++Pos)
		{
		double Value = Values[Pos];
		if (Value == DBL_MAX)
			{
			m_Density_ScaledValues.push_back(DBL_MAX);
			continue;
			}
		double ScaledValue = (Value - MinValue)/Range;
		asserta(ScaledValue >= 0 && ScaledValue <= 1);
		m_Density_ScaledValues.push_back(ScaledValue);
		}
	}

double DSS::GetDensity(uint Pos) const
	{
	const PDBChain &Chain = *m_Chain;
	const uint L = SIZE(Chain.m_Seq);
	if (Pos == 0 || Pos+1 >= L)
		return DBL_MAX;

	vector<double> PtCA;
	Chain.GetPt(Pos, PtCA);

	int iLo = int(Pos) - m_Density_W;
	if (iLo < 0)
		iLo = 0;
	int iHi = int(Pos) + m_Density_W;
	if (iHi >= int(L))
		iHi = int(L)-1;
	double D = 0;
	vector<double> Pt2;
	for (uint Pos2 = uint(iLo); Pos2 <= uint(iHi); ++Pos2)
		{
		if (Pos2 + m_Density_w >= Pos && Pos2 <= Pos + m_Density_w)
			continue;
		double Dist = Chain.GetDist(Pos, Pos2);
		double DistFactor = exp(-Dist/m_Density_Radius);
		D += DistFactor;
		}
	return D;
	}

void DSS::Get_NU_ND(uint Pos, double &NU, double &ND) const
	{
	NU = 0;
	ND = 0;
	const PDBChain &Chain = *m_Chain;
	const uint L = SIZE(Chain.m_Seq);
	if (Pos == 0 || Pos+1 >= L)
		{
		NU = DBL_MAX;
		ND = DBL_MAX;
		return;
		}

	vector<double> PtPrevCA;
	vector<double> PtNextCA;
	vector<double> PtCA;
	vector<double> PtCB;
	Chain.GetPt(Pos-1, PtPrevCA);
	Chain.GetPt(Pos, PtCA);
	Chain.GetPt(Pos+1, PtNextCA);

	vector<double> d1;
	vector<double> d2;
	Sub_Vecs(PtCA, PtPrevCA, d1);
	Sub_Vecs(PtCA, PtNextCA, d2);

	vector<double> VecPAB;
	Add_Vecs(d1, d2, VecPAB);
	NormalizeVec(VecPAB);

	vector<double> Pt2;
	vector<double> Vec12;
	//const int W = 50;
	//const double RADIUS = 20.0;
	int iLo = int(Pos) - m_NUDX_W;
	if (iLo < 0)
		iLo = 0;
	int iHi = int(Pos) + m_NUDX_W;
	if (iHi >= int(L))
		iHi = int(L)-1;
	for (uint Pos2 = uint(iLo); Pos2 <= uint(iHi); ++Pos2)
		{
		if (Pos2 + 3 >= Pos && Pos2 <= Pos + 3)
			continue;
		double Dist = Chain.GetDist(Pos, Pos2);
		double DistFactor = exp(-Dist/m_NU_ND_Radius);
		Chain.GetPt(Pos2, Pt2);
		Sub_Vecs(Pt2, PtCA, Vec12);
		double Theta = GetTheta_Vecs(VecPAB, Vec12);
		double Deg = degrees(Theta);
		if (Deg < 90)
			NU += DistFactor;
		else
			ND += DistFactor;
		}
	}

void DSS::Set_NU_ND_Vecs()
	{
	if (!m_NUs.empty())
		return;
	const uint L = GetSeqLength();
	m_NUs.reserve(L);
	m_NDs.reserve(L);
	m_NXs.reserve(L);
	for (uint Pos = 0; Pos < L; ++Pos)
		{
		double NU, ND;
		Get_NU_ND(Pos, NU, ND);
		m_NUs.push_back(NU);
		m_NDs.push_back(ND);
		m_NXs.push_back(NU+ND);
		}
	}

double DSS::GetFloat_NX(uint Pos)
	{
	Set_NU_ND_Vecs();
	return m_NXs[Pos];
	}

//double DSS::GetFloat_NU(uint Pos)
//	{
//	Set_NU_ND_Vecs();
//	return m_NUs[Pos];
//	}
//
//double DSS::GetFloat_ND(uint Pos)
//	{
//	Set_NU_ND_Vecs();
//	return m_NDs[Pos];
//	}

double DSS::GetSSDensity(uint Pos, char c)
	{
	SetSS();
	const PDBChain &Chain = *m_Chain;
	const uint L = SIZE(Chain.m_Seq);
	if (Pos == 0 || Pos+1 >= L)
		return DBL_MAX;

	vector<double> PtCA;
	Chain.GetPt(Pos, PtCA);

	int iLo = int(Pos) - m_SSDensity_W;
	if (iLo < 0)
		iLo = 0;
	int iHi = int(Pos) + m_SSDensity_W;
	if (iHi >= int(L))
		iHi = int(L)-1;
	double D = 0;
	double Dc = 0;
	vector<double> Pt2;
	for (uint Pos2 = uint(iLo); Pos2 <= uint(iHi); ++Pos2)
		{
		if (Pos2 + m_SSDensity_w >= Pos && Pos2 <= Pos + m_SSDensity_w)
			continue;
		char c2 = m_SS[Pos2];
		double Dist = Chain.GetDist(Pos, Pos2);
		double DistFactor = exp(-Dist/m_Density_Radius);
		D += DistFactor;
		if (c2 == c)
			Dc += DistFactor;
		}
	double r = Dc/(D + m_SSDensity_epsilon);
	return r;
	}

uint DSS::CalcREN(uint Pos, uint NEN) const
	{
	if (NEN == UINT_MAX)
		return UINT_MAX;

	const uint L = GetSeqLength();
	int iLo = INT_MAX;
	int iHi = INT_MAX;
	if (NEN > Pos)
		{
		iLo = int(Pos) - m_NEN_W;
		if (iLo < 0)
			iLo = 0;
		iHi = int(Pos) - 1;
		}
	else
		{
		iLo = int(Pos) + 1;
		iHi = int(Pos) + m_NEN_W;
		if (iHi >= int(L))
			iHi = int(L)-1;
		}
	if (iHi < 0)
		return UINT_MAX;
	if (iLo == INT_MAX || iHi == INT_MAX)
		return UINT_MAX;

	double MinDist = 999;
	uint MinPos = UINT_MAX;
	for (uint Pos2 = uint(iLo); Pos2 <= uint(iHi); ++Pos2)
		{
		if (Pos2 + m_NEN_w >= Pos && Pos2 <= Pos + m_NEN_w)
			continue;
		double Dist = m_Chain->GetDist(Pos, Pos2);
		if (Dist < MinDist)
			{
			MinDist = Dist;
			MinPos = Pos2;
			}
		}
	return MinPos;
	}

uint DSS::CalcNEN(uint Pos) const
	{
	const uint L = GetSeqLength();
	int iLo = int(Pos) - m_NEN_W;
	if (iLo < 0)
		iLo = 0;
	int iHi = int(Pos) + m_NEN_W;
	if (iHi >= int(L))
		iHi = int(L)-1;
	double MinDist = 999;
	uint MinPos = UINT_MAX;
	for (uint Pos2 = uint(iLo); Pos2 <= uint(iHi); ++Pos2)
		{
		if (Pos2 + m_NEN_w >= Pos && Pos2 <= Pos + m_NEN_w)
			continue;
		double Dist = m_Chain->GetDist(Pos, Pos2);
		if (Dist < MinDist)
			{
			MinDist = Dist;
			MinPos = Pos2;
			}
		}
	return MinPos;
	}

uint DSS::GetNEN(uint Pos)
	{
	SetNENs();
	asserta(Pos < SIZE(m_NENs));
	return m_NENs[Pos];
	}

uint DSS::GetREN(uint Pos)
	{
	SetNENs();
	asserta(Pos < SIZE(m_RENs));
	return m_RENs[Pos];
	}

void DSS::SetNENs()
	{
	if (!m_NENs.empty())
		return;
	const uint L = GetSeqLength();
	m_NENs.reserve(L);
	m_RENs.reserve(L);
	for (uint Pos = 0; Pos < L; ++Pos)
		{
		uint NEN = CalcNEN(Pos);
		uint REN = CalcREN(Pos, NEN);
		m_NENs.push_back(NEN);
		m_RENs.push_back(REN);
		}
	}

uint DSS::Get_NENSS(uint Pos)
	{
	SetSS();
	SetNENs();
	uint NEN = GetNEN(Pos);
	if (NEN == UINT_MAX)
		return SSCharToInt('~');
	asserta(NEN < SIZE(m_SS));
	char c = m_SS[NEN];
	return SSCharToInt(c);
	}

uint DSS::Get_RENSS(uint Pos)
	{
	SetSS();
	SetNENs();
	uint NEN = GetREN(Pos);
	if (NEN == UINT_MAX)
		return SSCharToInt('~');
	asserta(NEN < SIZE(m_SS));
	char c = m_SS[NEN];
	return SSCharToInt(c);
	}

double DSS::GetFloat_NENDist(uint Pos)
	{
	uint NEN = GetNEN(Pos);
	if (NEN == UINT_MAX)
		return m_DefaultNENDist;
	double d = m_Chain->GetDist(Pos, NEN);
	return d;
	}

double DSS::GetFloat_PMDist(uint Pos)
	{
	int iPos = (int) Pos;
	int L = (int) GetSeqLength();
	if (L < 8)
		return 0;
	int Pos1 = iPos - m_PMDelta;
	int Pos2 = iPos + m_PMDelta;
	if (Pos1 < 0)
		Pos1 = 0;
	if (Pos2 >= L)
		Pos2 = L - 1;
	double d = m_Chain->GetDist(uint(Pos1), uint(Pos2));
	return d;
	}

double DSS::GetFloat_RENDist(uint Pos)
	{
	uint NEN = GetREN(Pos);
	if (NEN == UINT_MAX)
		return m_DefaultNENDist;
	double d = m_Chain->GetDist(Pos, NEN);
	return d;
	}

uint DSS::Get_NormDens4(uint Pos)
	{
	uint ND = GetFeature(FEATURE_NormDens, Pos);
	if (ND == UINT_MAX)
		return WILDCARD;
	asserta(ND < 16);
	return ND/4;
	}

uint DSS::Get_NENDist4(uint Pos)
	{
	uint ND = GetFeature(FEATURE_NENDist, Pos);
	if (ND == UINT_MAX)
		return WILDCARD;
	asserta(ND < 16);
	return ND/4;
	}

uint DSS::Get_RENDist4(uint Pos)
	{
	uint ND = GetFeature(FEATURE_RENDist, Pos);
	if (ND == UINT_MAX)
		return WILDCARD;
	asserta(ND < 16);
	return ND/4;
	}

// ADEHKNPQRST,CFILMVWY,G
uint DSS::Get_AA3(uint Pos)
	{
	const string &Seq = m_Chain->m_Seq;
	asserta(Pos < SIZE(Seq));
	char c = Seq[Pos];
	if (c == 'G')
		return 0;
	if (strchr("ADEHKNPQRST", c) != 0)
		return 1;
	if (strchr("CFILMVWY", c) != 0)
		return 2;
	return WILDCARD;
	}

// AHPST,CFILMVWY,DEKNQR,G
uint DSS::Get_AA4(uint Pos)
	{
	const string &Seq = m_Chain->m_Seq;
	asserta(Pos < SIZE(Seq));
	char c = Seq[Pos];
	if (c == 'G')
		return 0;
	if (strchr("AHPST", c) != 0)
		return 1;
	if (strchr("CFILMVWY", c) != 0)
		return 2;
	if (strchr("DEKNQR", c) != 0)
		return 3;
	return WILDCARD;
	}

void DSS::GetMuLetters(uint MuLetter, vector<uint> &Letters) const
	{
	Letters.clear();
	uint n = SIZE(m_Params->m_MuFeatures);
	if (MuLetter == UINT_MAX)
		{
		for (uint i = 0; i < n; ++i)
			Letters.push_back(UINT_MAX);
		return;
		}

	uint CL = MuLetter;
	uint m = 1;
	for (uint i = 0; i < n; ++i)
		{
		uint m = m_Params->m_MuAlphaSizes[i];
		uint Letter = CL%m;
		Letters.push_back(Letter);
		CL /= m;
		}
	}

uint DSS::GetMuLetter(const vector<uint> &Letters) const
	{
	uint n = SIZE(m_Params->m_MuFeatures);
	asserta(SIZE(Letters) == n);
	uint MuLetter = 0;
	uint m = 1;
	for (uint i = 0; i < n; ++i)
		{
		uint Letter = Letters[i];
		if (Letter == UINT_MAX)
			return UINT_MAX;
		MuLetter = MuLetter + m*Letter;
		m *= m_Params->m_MuAlphaSizes[i];
		}
	asserta(MuLetter < m_Params->m_MuAlphaSize);
	return MuLetter;
	}

uint DSS::Get_Mu(uint Pos)
	{
	uint MuLetter = 0;
	uint m = 1;
	for (uint i = 0; i < SIZE(m_Params->m_MuFeatures); ++i)
		{
		uint Letter = GetFeature(m_Params->m_MuFeatures[i], Pos);
		if (Letter == UINT_MAX)
			return UINT_MAX;
		MuLetter = MuLetter + m*Letter;
		m *= m_Params->m_MuAlphaSizes[i];
		}

	asserta(MuLetter < m_Params->m_MuAlphaSize);
	return MuLetter;
	}

void DSS::GetMuLetters(vector<uint> &Letters)
	{
	Letters.clear();
	const uint L = GetSeqLength();
	Letters.reserve(L);
	for (uint Pos = 0; Pos < L; ++Pos)
		{
		uint Letter = GetFeature(FEATURE_Mu, Pos);
		asserta(Letter < m_PatternAlphaSize1 || Letter == UINT_MAX);
		Letters.push_back(Letter);
		}
	}

void DSS::GetAaKmers(const vector<byte> &Letters,
  vector<uint> &Kmers)
	{
	Kmers.clear();
	const string PatternStr = m_Params->m_PatternStr;
	const uint PatternLength = SIZE(PatternStr);
	const uint L = SIZE(Letters);
	Kmers.reserve(L);
	for (uint Pos = 0; Pos + PatternLength <= L; ++Pos)
		{
		uint Kmer = 0;
		for (uint j = 0; j < PatternLength; ++j)
			{
			if (PatternStr[j] == '1')
				{
				asserta(Pos + j < SIZE(Letters));
				uint Letter = Letters[Pos + j];
				assert(Letter < 20);
				Kmer = Kmer*20 + Letter;
				}
			}
		if (Kmer != UINT_MAX)
			Kmers.push_back(Kmer);
		}
	}

void DSS::GetMuKmers(const vector<byte> &Letters,
  vector<uint> &Kmers)
	{
	Kmers.clear();
	const string PatternStr = m_Params->m_PatternStr;
	const uint PatternLength = SIZE(PatternStr);
	const uint L = SIZE(Letters);
	Kmers.reserve(L);
	for (uint Pos = 0; Pos + PatternLength <= L; ++Pos)
		{
		uint Kmer = 0;
		for (uint j = 0; j < PatternLength; ++j)
			{
			if (PatternStr[j] == '1')
				{
				asserta(Pos + j < SIZE(Letters));
				uint Letter = Letters[Pos + j];
				//if (Letter == UINT_MAX)
				//	{
				//	Kmer = UINT_MAX;
				//	break;
				//	}
				assert(Letter < 36);
				Kmer = Kmer*m_PatternAlphaSize1 + Letter;
				}
			}
		//if (Kmer != UINT_MAX)
		assert(Kmer != UINT_MAX);
		Kmers.push_back(Kmer);
		}
	}

void DSS::GetAaLetters(vector<byte> &Letters)
	{
	Letters.clear();
	const uint L = GetSeqLength();
	Letters.reserve(L);
	const string &Seq = m_Chain->m_Seq;
	for (uint Pos = 0; Pos < L; ++Pos)
		{
		char c = Seq[Pos];
		uint Letter = g_CharToLetterAmino[c];
		if (Letter >= 20)
			Letter = 0;
		Letters.push_back(byte(Letter));
		}
	}

void DSS::GetMuLetters(vector<byte> &Letters)
	{
	Letters.clear();
	const uint L = GetSeqLength();
	Letters.reserve(L);
	for (uint Pos = 0; Pos < L; ++Pos)
		{
		uint Letter = Get_Mu(Pos);
		if (Letter == UINT_MAX)
			Letter = 0;
		asserta(Letter < 256);
		asserta(Letter < 36);
		Letters.push_back(byte(Letter));
		}
	}

void DSS::GetProfile(vector<vector<byte> > &Profile)
	{
	Profile.clear();
	const uint L = GetSeqLength();
	const string &Seq = m_Chain->m_Seq;
	const uint FeatureCount = m_Params->GetFeatureCount();
	Profile.reserve(FeatureCount);
	for (uint i = 0; i < FeatureCount; ++i)
		{
		vector<byte> ProfRow;
		ProfRow.reserve(L);
		FEATURE Feature = m_Params->m_Features[i];
		for (uint Pos = 0; Pos < L; ++Pos)
			{
			uint Letter = GetFeature(Feature, Pos);
			if (Letter == UINT_MAX)
				ProfRow.push_back(31);
			else
				{
				asserta(Letter < 31);
				ProfRow.push_back(byte(Letter));
				}
			}
		Profile.push_back(ProfRow);
		}
	}

double DSS::GetFloatFeature(uint FeatureIndex, uint Pos)
	{
	switch (FeatureIndex)
		{
#define F(x)	case FEATURE_##x: return GetFloat_##x(Pos);
#include "floatfeatures.h"
#undef F
		}
	asserta(false);
	return DBL_MAX;
	}

uint DSS::GetAlphaSize(FEATURE F)
	{
	switch (F)
		{
	case FEATURE_AA:
		return 20;

	case FEATURE_SS:
	case FEATURE_NENSS:
	case FEATURE_RENSS:
	case FEATURE_NormDens4:
	case FEATURE_NENDist4:
	case FEATURE_RENDist4:
	case FEATURE_AA4:
		return 4;

	case FEATURE_SS3:
	case FEATURE_NENSS3:
	case FEATURE_RENSS3:
	case FEATURE_AA3:
		return 3;

	case FEATURE_Conf:
	case FEATURE_NENConf:
	case FEATURE_RENConf:
	case FEATURE_NormDens:
	case FEATURE_NENDist:
	case FEATURE_RENDist:
	case FEATURE_HelixDens:
	case FEATURE_StrandDens:
	case FEATURE_DstNxtHlx:
	case FEATURE_DstPrvHlx:
	case FEATURE_NX:
	case FEATURE_PMDist:
		return 16;

	case FEATURE_Mu:
		return DSSParams::m_MuAlphaSize;
		}
	Die("GetAlphaSize(%s)", FeatureToStr(F));
	return UINT_MAX;
	}

void DSS::SetParams(const DSSParams &Params)
	{
	m_Params = &Params;
	const uint AS = GetAlphaSize(FEATURE_Mu);
	m_PatternAlphaSize1 = AS;
	uint PatternOnes = GetPatternOnes(m_Params->m_PatternStr);
	m_PatternAlphaSize = myipow(AS, PatternOnes);
	}

uint DSS::GetFeature(FEATURE Feature, uint Pos)
	{
	return GetFeature(uint(Feature), Pos);
	}

uint DSS::GetFeature(uint FeatureIndex, uint Pos)
	{
	switch (FeatureIndex)
		{
		case FEATURE_AA:
			{
			char AminoChar = m_Chain->m_Seq[Pos];
			uint AminoLetter = g_CharToLetterAmino[AminoChar];
			if (AminoLetter >= 20)
				return WILDCARD;
			return AminoLetter;
			}

#define F(x)	case FEATURE_##x: return Get_##x(Pos);
#include "intfeatures.h"
#undef F

#define F(x)	case FEATURE_##x: \
		{ \
		double Value = GetFloat_##x(Pos); \
		return ValueToInt_##x(Value); \
		}
#include "floatfeatures.h"
#undef F

	default:
		break;
		}
	asserta(false);
	return UINT_MAX;
	}

uint DSS::ValueToInt(const vector<double> &Ts, double Value)
	{
	const uint N = SIZE(Ts);
	for (uint i = 0; i < N; ++i)
		if (Value <= Ts[i])
			return i;
	return N;
	}

double DSS::GetFloat_DstPrvHlx(uint Pos)
	{
	SetSSEs();
	const uint SSECount = SIZE(m_SSE_Mids);
	for (uint i = 0; i < SSECount; ++i)
		{
		if (m_SSE_cs[SSECount-i-1] != 'h')
			continue;
		uint Mid = m_SSE_Mids[i];
		if (Mid + m_SSE_Margin >= Pos)
			continue;
		double Dist = m_Chain->GetDist(Pos, Mid);
		return Dist;
		}
	return 0;
	}

double DSS::GetFloat_DstNxtHlx(uint Pos)
	{
	SetSSEs();
	const uint SSECount = SIZE(m_SSE_Mids);
	for (uint i = 0; i < SSECount; ++i)
		{
		if (m_SSE_cs[i] != 'h')
			continue;
		uint Mid = m_SSE_Mids[i];
		if (Mid <= Pos + m_SSE_Margin)
			continue;
		double Dist = m_Chain->GetDist(Pos, Mid);
		return Dist;
		}
	return 0;
	}
