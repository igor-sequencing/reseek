#include "myutils.h"
#include "seqdb.h"
#include "pdbchain.h"
#include "daliscorer.h"

void cmd_lddt_msas()
	{
	asserta(optset_input);
	asserta(optset_testdir);
	FILE* fOut = CreateStdioFile(opt(output));
	const bool MissingSeqOk = opt(missingtestseqok);

	string TestDir = string(opt(testdir));
	Dirize(TestDir);

	DALIScorer DS;
	DS.LoadChains(opt(input));

	vector<string> Accs;
	ReadLinesFromFile(g_Arg1, Accs);

	const bool DoCore = opt(core);

	const uint N = SIZE(Accs);
	double Sum_Z = 0;
	double Sum_Z15 = 0;
	double Sum_LDDT_mu = 0;
	double Sum_LDDT_fm = 0;
	uint FoundFileCount = 0;
	for (uint i = 0; i < N; ++i)
		{
		const string &Acc = Accs[i];
		const string &FN = TestDir + Acc;
		if (!StdioFileExists(FN))
			{
			fprintf(fOut, "missing_aln=%s\n", FN.c_str());
			continue;
			}
		++FoundFileCount;

		SeqDB MSA;
		MSA.FromFasta(FN, true);

		ProgressStep(i, N, "%s", Acc.c_str());
		bool Ok = DS.SetMSA(Acc, MSA, DoCore, MissingSeqOk);
		if (!Ok)
			continue;

		double Z = DS.GetZ();
		double LDDT_mu = DS.GetLDDT_muscle();
		uint CoreColCount = DS.m_CoreColCount;

		Sum_LDDT_mu += LDDT_mu;
		if (fOut != 0)
			{
			fprintf(fOut, "aln=%s", FN.c_str());
			fprintf(fOut, "\tLDDT_mu=%.4f", LDDT_mu);
			if (DoCore)
				fprintf(fOut, "\tnr_core_cols=%u", CoreColCount);
			fprintf(fOut, "\n");
			}
		}

	double Mean_LDDT_mu = 0;
	if (FoundFileCount > 0)
		Mean_LDDT_mu = Sum_LDDT_mu/FoundFileCount;

	if (fOut != 0)
		{
		fprintf(fOut, "testdir=%s", TestDir.c_str());
		fprintf(fOut, "\tavg_LDDT_mu=%.4f", Mean_LDDT_mu);
		fprintf(fOut, "\n");
		}
	  
	CloseStdioFile(fOut);

	ProgressLog("MSAs=%u/%u", FoundFileCount, N);
	ProgressLog(" LDDT_mu=%.4f", Mean_LDDT_mu);
	ProgressLog("\n");
	}
