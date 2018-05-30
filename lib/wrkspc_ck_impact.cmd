setMode -bs
setMode -bs
setMode -bs
setMode -bs
setCable -port auto
Identify -inferir
identifyMPM
assignFile -p 1 -file "Bitfiles/scrod_top_A5_KLM_9UMB_151019_csp.bit"
Program -p 1
setMode -bs
setMode -bs
deleteDevice -position 1
setMode -bs
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
