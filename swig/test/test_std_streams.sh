#!/bin/sh

TOOLDIR=../../tools/src/
STRINGS2FST="$TOOLDIR"hfst-strings2fst
COMPARE="$TOOLDIR"hfst-compare" -s"

PYTHON=python
if [ "$1" = "--python" ]; then
    PYTHON=$2;
fi

STRING1="abcdefghijklmnopqrstuvwxyz"
STRING2="ABCDEFGHIJKLMNOPQRSTUVWXYZ"

echo $STRING1 | $STRINGS2FST -f sfst > tr1.sfst
echo $STR1NG2 | $STRINGS2FST -f sfst > tr2.sfst
cat tr1.sfst tr2.sfst > tr.sfst
rm tr1.sfst tr2.sfst

echo $STRING1 | $STRINGS2FST -f openfst-tropical > tr1.ofst
echo $STRING2 | $STRINGS2FST -f openfst-tropical > tr2.ofst
cat tr1.ofst tr2.ofst > tr.ofst
rm tr1.ofst tr2.ofst

echo $STRING1 | $STRINGS2FST -f foma > tr1.foma
echo $STRING2 | $STRINGS2FST -f foma > tr2.foma
cat tr1.foma tr2.foma > tr.foma
rm tr1.foma tr2.foma

if ! (cat tr.sfst | PYTHONPATH=$PWD/.. $PYTHON ./test_std_streams.py sfst 2 > tmp.sfst); then
    echo "ERROR: in sfst"
    rm tr.sfst
    exit 1
fi

if ! ($COMPARE tr.sfst tmp.sfst); then
    echo "ERROR in sfst"
    rm tr.sfst tmp.sfst
    exit 1
fi
rm tr.sfst tmp.sfst

if ! (cat tr.ofst | PYTHONPATH=$PWD/.. $PYTHON ./test_std_streams.py openfst 2 > tmp.ofst); then
    echo "ERROR: in openfst"
    rm tr.ofst
    exit 1
fi

if ! ($COMPARE tr.ofst tmp.ofst); then
    echo "ERROR: in openfst"
    rm tr.ofst tmp.ofst
    exit 1
fi
rm tr.ofst tmp.ofst

if ! (cat tr.foma | PYTHONPATH=$PWD/.. $PYTHON ./test_std_streams.py foma 2 > tmp.foma); then
    rm tr.foma
    echo "ERROR: in foma"
    exit 1
fi  

if ! ($COMPARE tr.foma tmp.foma); then
    echo "ERROR: in foma"
    rm tr.foma tmp.foma
    exit 1
fi  
rm tr.foma tmp.foma