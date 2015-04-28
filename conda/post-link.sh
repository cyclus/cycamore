
echo "post-link.sh, PREFIX: $PREFIX"

mv $PREFIX/bin/cycamore_unit_tests $PREFIX/bin/cycamore_unit_tests_base
echo "
#!/bin/bash
export LD_LIBRARY_PATH=$PREFIX/lib:$PREFIX/lib/cyclus
 export DYLD_FALLBACK_LIBRARY_PATH=$PREFIX/lib/cyclus
export CYCLUS_NUC_DATA=$PREFIX/share/cyclus/cyclus_nuc_data.h5
export CYCLUS_PATH=$PREFIX/lib/cyclus
export CYCLUS_RNG_SCHEMA=$PREFIX/share/cyclus/cyclus.rng.in
export DYLD_LIBRARY_PATH=
$PREFIX/bin/cycamore_unit_tests_base \$*

" > $PREFIX/bin/cycamore_unit_tests
chmod 755 $PREFIX/bin/cycamore_unit_tests

