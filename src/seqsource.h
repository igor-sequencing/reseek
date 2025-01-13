#ifndef seqsource_h
#define seqsource_h

#include <mutex>
#include "objmgr.h"

class SeqInfo;

class SeqSource
	{
	mutex m_Lock;

public:
	bool m_DoGetLock;
	ObjMgr m_OM;

protected:
	SeqInfo *m_SI;
	unsigned m_SeqCount;

public:
	SeqSource();
	virtual ~SeqSource();

public:
	virtual bool GetIsNucleo() = 0;
	virtual unsigned GetPctDoneX10() = 0;
	virtual const char *GetFileNameC() const = 0;
	virtual void Rewind() = 0;
	virtual bool GetPctDone_Supported() const { return false; }
	virtual double GetPctDone() const { return -1; }

public:
	virtual bool GetNextLo(SeqInfo *SI) = 0;

public:
	bool GetNext(SeqInfo *SI);
	};

SeqSource *MakeSeqSource(const string &FileName);

#endif // seqsource_h
