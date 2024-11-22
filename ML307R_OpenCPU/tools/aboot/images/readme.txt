dsp:
1606S/1606C：ASR1606/dsp.bin(DSP_Normal)
1606L/1606C_SS/1606S_SS：ASR1606/dsp_reduce.bin(DSP_Reduce_SuLog)
1602S：ASR1602/dsp.bin(DSP_WiFiScan_Reduce_SuLog_Buffer)//支持wifiscan功能产品必须用此
1602C：ASR1602/dsp_reduce.bin(DSP_Reduce_SuLog_Buffer)

boot33:
1602S/1602C/1606L/1606C_SS/1606S_SS: boot33_reduce.bin(boot33_craneL_SingleSim.bin)
1606S/1606C: boot33.bin(boot33_craneL.bin)

logo:
1602S/1602C/1606L/1606C_SS/1606S_SS: 无此分区
1606S/1606C: logo.bin(logo_no_dsp_adc.bin)

logo:
1602S/1602C/1606L/1606C_SS/1606S_SS/1606S/1606C: updater.bin(updater_no_dsp_adc.bin)//有修改，需编译生成

apn:
1606S/1606C: apn.bin
1602S/1602C/1606L/1606C_SS/1606S_SS: 无此分区，移植到代码