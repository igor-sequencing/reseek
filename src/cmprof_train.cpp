#include "myutils.h"
#include "cmprof.h"
#include "chainreader2.h"
#include "alpha.h"

void cmd_cmprof_train()
	{
	const string &ChainFN = opt(cmprof_train);
	const string &MSAFN = opt(input);

	SeqDB MSA;
	MSA.FromFasta(MSAFN, true);
	const uint SeqCount = MSA.GetSeqCount();

	ChainReader2 CR;
	CR.Open(ChainFN);

	CMProf Prof;
	Prof.SetMSA(MSA);
	ProgressLog("%u core cols\n", SIZE(Prof.m_CoreCols));

	uint FoundCount = 0;
	uint NotFoundCount = 0;
	for (;;)
		{
		PDBChain *Q = CR.GetNext();
		if (Q == 0)
			break;
		bool Found = Prof.TrainChain(*Q);
		if (Found)
			++FoundCount;
		else
			++NotFoundCount;
		}
	Progress("%u / %u structures matched\n", SeqCount, FoundCount);
	if (FoundCount == 0)
		Die("No structures found");
	if (NotFoundCount > 0)
		Warning("%u sequences not found", NotFoundCount);
	Prof.FinalizeTrain();
	Prof.ToFile(opt(output));
	}
