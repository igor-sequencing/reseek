#!/usr/bin/python3

import sys

errors = 0

def read_fasta(fn):
    seqdict = {}
    label = None
    seq = ""
    for line in open(fn):
        line = line.strip()
        if len(line) == 0:
            continue
        if line[0] == ">":
            if len(seq) > 0:
                seqdict[label] = seq
            label = line[1:]
            seq = ""
        else:
            seq += line
    seqdict[label] = seq
    return seqdict

chains = read_fasta("../test_data/chains.fa")
labels = set(chains.keys())

# 54M3D358M
def split_cigar(cigar):
    global errors
    n = 0
    op = None
    ns = []
    ops = []
    for c in cigar:
        if c.isdigit():
            n = n*10 + int(c)
        else:
            op = c
            if not op in "MDI":
                errors += 1
                print("%s: ERROR bad cigar #3 %s" % (sys.argv[0], cigar))
                return
            ops.append(op)
            ns.append(n)
            n = 0
    if not op in "MDI":
        errors += 1
        print("%s: ERROR bad cigar #4 " % (sys.argv[0], cigar))
        return
    ops.append(op)
    ns.append(n)
    assert len(ops) == len(ns)
    return ns, ops

def cigar2path(cigar):
    ns, ops = split_cigar(cigar)
    path = ""
    N = len(ns)
    for i in range(N):
        path += ops[i]*ns[i]
    return path

def check_default(fn):
    global errors
    for line in open(fn):
        flds = line[:-1].split('\t')
        if len(flds) != 5:
            print("%s: ERROR %s not five flds" % (sys.argv[0], fn))
            errors += 1
            return
        try:
            Q = float(flds[0])
        except:
            print("%s: ERROR %s aq %s not float" % (sys.argv[0], fn, flds[0]))
            errors += 1
            return
        try:
            E = float(flds[3])
        except:
            print("%s: ERROR %s evalue %s not float" % (sys.argv[0], fn, flds[0]))
            errors += 1
            return
        if not flds[1] in labels:
            print("%s: ERROR %s label %s not found" % (sys.argv[0], fn, flds[1]))
            errors += 1
            return
        if not flds[2] in labels:
            print("%s: ERROR %s label %s not found" % (sys.argv[0], fn, flds[2]))
            errors += 1
            return
            
    print("ok " + fn)

def check_local_row(fn, labelfld, lofld, hifld, cigarfld, op, rowfld):
    global errors
    for line in open(fn):
        try:
            flds = line[:-1].split('\t')
            label = flds[labelfld]
            lo = int(flds[lofld])
            hi = int(flds[hifld])
            cigar = flds[cigarfld]
            row = flds[rowfld]
            chain = chains[label]
        except:
            errors += 1
            print("%s: ERROR %s exception in check_local_row" % (sys.argv[0], fn))
            return
        chainseg = chain[lo-1:hi]
        seg = ""
        for c in row:
            if c != '.' and c != '-':
                seg += c
        if seg != chainseg:
            errors += 1
            print(seg)
            print(chainseg)
            print("     seg=" + seg)
            print("chainseg=" + chainseg)
            print("%s: ERROR %s %s seg mismatch #1" % (sys.argv[0], fn, label))
            return
        path = cigar2path(cigar)
        cols = len(path)
        rowlen = len(row)
        if cols != rowlen:
            errors += 1
            print("%s: ERROR %s %s len path %d != cols %d" % (sys.argv[0], fn, label, cols, rowlen))
            return
        for col in range(cols):
            c = path[col]
            if c == 'M' or c == op:
                if not row[col].isupper():
                    errors += 1
                    print("ERROR %s %s row op" % (fn, label))
                    return
            
    print("ok check_local_row " + fn)       

d = "../test_output/"
check_default(d + "columns_default.tsv")
check_default(d + "columns_same_as_default.tsv")

#                0      1   2   3  4   5   6  7     8    9   10
#	-columns query+target+qlo+qhi+ql+tlo+thi+tl+cigar+qrow+trow
check_local_row(d + "columns_local_rows.tsv", 0, 2, 3, 8, 'D', 9)
check_local_row(d + "columns_local_rows.tsv", 1, 5, 6, 8, 'I', 10)

exit(1 if errors > 0 else 0)
