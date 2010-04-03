Translation files generation:
lupdate -verbose ../src/src.pro
linguist qmp3gain_hu.ts
lrelease -verbose ../src/src.pro	or	make
