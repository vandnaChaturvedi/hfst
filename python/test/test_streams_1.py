import hfst

import sys
if sys.argv[1] == 'sfst':
    if not hfst.HfstTransducer.is_implementation_type_available(hfst.SFST_TYPE):
        sys.exit(77)
    hfst.set_default_fst_type(hfst.SFST_TYPE)
elif sys.argv[1] == 'foma':
    if not hfst.HfstTransducer.is_implementation_type_available(hfst.FOMA_TYPE):
        sys.exit(77)
    hfst.set_default_fst_type(hfst.FOMA_TYPE)
elif sys.argv[1] == 'openfst':
    if not hfst.HfstTransducer.is_implementation_type_available(hfst.TROPICAL_OPENFST_TYPE):
        sys.exit(77)
    hfst.set_default_fst_type(hfst.TROPICAL_OPENFST_TYPE)
else:
    raise RuntimeError('implementation format not recognized')

tr1 = hfst.regex('föö:bär')
tr2 = hfst.regex('0')
tr3 = hfst.regex('0-0')

ostr = hfst.HfstOutputStream()
ostr.write(tr1)
ostr.write(tr2)
ostr.write(tr3)
ostr.flush()
ostr.close()
