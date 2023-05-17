#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=arm-brcm-linux-gnueabi-gcc
CCC=arm-brcm-linux-gnueabi-g++
CXX=arm-brcm-linux-gnueabi-g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=Cygwin-Windows
CND_DLIB_EXT=dll
CND_CONF=Fubon_of_V3
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include NbMakefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/CMAS/CMASAPIISO.o \
	${OBJECTDIR}/CMAS/CMASFunction.o \
	${OBJECTDIR}/CMAS/CMASMenu.o \
	${OBJECTDIR}/CMAS/CMASprtByBuffer.o \
	${OBJECTDIR}/CMAS/CMASsrc.o \
	${OBJECTDIR}/CTLS/CTLS.o \
	${OBJECTDIR}/ECC/ECC.o \
	${OBJECTDIR}/ECC/ICER/APDU.o \
	${OBJECTDIR}/ECC/ICER/BLC.o \
	${OBJECTDIR}/ECC/ICER/BankData.o \
	${OBJECTDIR}/ECC/ICER/CMAS_APDU.o \
	${OBJECTDIR}/ECC/ICER/CMAS_TM.o \
	${OBJECTDIR}/ECC/ICER/CallAPI.o \
	${OBJECTDIR}/ECC/ICER/Com.o \
	${OBJECTDIR}/ECC/ICER/Function.o \
	${OBJECTDIR}/ECC/ICER/ICERAPI.o \
	${OBJECTDIR}/ECC/ICER/ICERAes.o \
	${OBJECTDIR}/ECC/ICER/ICERLib.o \
	${OBJECTDIR}/ECC/ICER/SIS2.o \
	${OBJECTDIR}/ECC/ICER/SIS2_2.o \
	${OBJECTDIR}/ECC/ICER/SSL.o \
	${OBJECTDIR}/ECC/ICER/TCPIP.o \
	${OBJECTDIR}/ECC/ICER/TM.o \
	${OBJECTDIR}/ECC/ICER/XMLFunc.o \
	${OBJECTDIR}/EMVSRC/EMVsrc.o \
	${OBJECTDIR}/EMVSRC/EMVxml.o \
	${OBJECTDIR}/FUBON/FUBONCostoEcr.o \
	${OBJECTDIR}/FUBON/FUBONEcr.o \
	${OBJECTDIR}/FUBON/FUBONTablePack.o \
	${OBJECTDIR}/FUBON/FUBONencryptFunc.o \
	${OBJECTDIR}/FUBON/FUBONfunc.o \
	${OBJECTDIR}/FUBON/FUBONiso.o \
	${OBJECTDIR}/FUBON/FUBONtsam.o \
	${OBJECTDIR}/JSON/cJSON.o \
	${OBJECTDIR}/NCCC/NCCCTicketSrc.o \
	${OBJECTDIR}/NCCC/NCCCtSAM.o \
	${OBJECTDIR}/SOURCE/COMM/Bluetooth.o \
	${OBJECTDIR}/SOURCE/COMM/Comm.o \
	${OBJECTDIR}/SOURCE/COMM/Ethernet.o \
	${OBJECTDIR}/SOURCE/COMM/Ftps.o \
	${OBJECTDIR}/SOURCE/COMM/GPRS.o \
	${OBJECTDIR}/SOURCE/COMM/GSM.o \
	${OBJECTDIR}/SOURCE/COMM/Modem.o \
	${OBJECTDIR}/SOURCE/COMM/TLS.o \
	${OBJECTDIR}/SOURCE/COMM/WiFi.o \
	${OBJECTDIR}/SOURCE/DISPLAY/DisTouch.o \
	${OBJECTDIR}/SOURCE/DISPLAY/Display.o \
	${OBJECTDIR}/SOURCE/EVENT/CustomerMenu.o \
	${OBJECTDIR}/SOURCE/EVENT/Event.o \
	${OBJECTDIR}/SOURCE/EVENT/EventDispFunc.o \
	${OBJECTDIR}/SOURCE/EVENT/Flow.o \
	${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer0Menu.o \
	${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer1Menu.o \
	${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer2Menu.o \
	${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer3Menu.o \
	${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer4Menu.o \
	${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer5Menu.o \
	${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer6Menu.o \
	${OBJECTDIR}/SOURCE/EVENT/Menu.o \
	${OBJECTDIR}/SOURCE/FUNCTION/APDU.o \
	${OBJECTDIR}/SOURCE/FUNCTION/ASMC.o \
	${OBJECTDIR}/SOURCE/FUNCTION/AccountFunction/DispBillInfo.o \
	${OBJECTDIR}/SOURCE/FUNCTION/AccountFunction/PrintBillInfo.o \
	${OBJECTDIR}/SOURCE/FUNCTION/Accum.o \
	${OBJECTDIR}/SOURCE/FUNCTION/BaseUSB.o \
	${OBJECTDIR}/SOURCE/FUNCTION/Batch.o \
	${OBJECTDIR}/SOURCE/FUNCTION/CARD_FUNC/CardFunction.o \
	${OBJECTDIR}/SOURCE/FUNCTION/CDT.o \
	${OBJECTDIR}/SOURCE/FUNCTION/CFGT.o \
	${OBJECTDIR}/SOURCE/FUNCTION/COSTCO_FUNC/Costco.o \
	${OBJECTDIR}/SOURCE/FUNCTION/CPT.o \
	${OBJECTDIR}/SOURCE/FUNCTION/CPT_Backup.o \
	${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC/CreditCheckFunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC/CreditFlowDispFunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC/CreditPrintFunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC/CreditProcessDataFunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC/Creditfunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/CUP_FUNC/CupCheckFunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/CUP_FUNC/CupFlowDispFunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/CUP_FUNC/CupProcessDataFunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/Card.o \
	${OBJECTDIR}/SOURCE/FUNCTION/ECCDT.o \
	${OBJECTDIR}/SOURCE/FUNCTION/ECR_FUNC/ECR.o \
	${OBJECTDIR}/SOURCE/FUNCTION/ECR_FUNC/EcrPackFunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/ECR_FUNC/EcrUnPackFunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/ECR_FUNC/RS232.o \
	${OBJECTDIR}/SOURCE/FUNCTION/EDC.o \
	${OBJECTDIR}/SOURCE/FUNCTION/EDC_Para_Table_Func.o \
	${OBJECTDIR}/SOURCE/FUNCTION/EST.o \
	${OBJECTDIR}/SOURCE/FUNCTION/FILE_FUNC/FIleLogFunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/FILE_FUNC/File.o \
	${OBJECTDIR}/SOURCE/FUNCTION/FuncTable.o \
	${OBJECTDIR}/SOURCE/FUNCTION/Function.o \
	${OBJECTDIR}/SOURCE/FUNCTION/HDPT.o \
	${OBJECTDIR}/SOURCE/FUNCTION/HDT.o \
	${OBJECTDIR}/SOURCE/FUNCTION/INSTALL_FUNC/InstCheckFunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/INSTALL_FUNC/InstProcessDataFunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/IPASSDT.o \
	${OBJECTDIR}/SOURCE/FUNCTION/KMS.o \
	${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC/ExDevicePackData.o \
	${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC/ExDeviceUnPackData.o \
	${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC/JsonMultiHostFunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC/MultiFunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC/MultiHostFunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/MVT.o \
	${OBJECTDIR}/SOURCE/FUNCTION/PCD.o \
	${OBJECTDIR}/SOURCE/FUNCTION/PIT.o \
	${OBJECTDIR}/SOURCE/FUNCTION/PWD.o \
	${OBJECTDIR}/SOURCE/FUNCTION/PowerManagement.o \
	${OBJECTDIR}/SOURCE/FUNCTION/QAT.o \
	${OBJECTDIR}/SOURCE/FUNCTION/REDEEM_FUNC/RedeemCheckFunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/REDEEM_FUNC/RedeemProcessDataFunc.o \
	${OBJECTDIR}/SOURCE/FUNCTION/SCDT.o \
	${OBJECTDIR}/SOURCE/FUNCTION/SKM.o \
	${OBJECTDIR}/SOURCE/FUNCTION/Signpad.o \
	${OBJECTDIR}/SOURCE/FUNCTION/Sqlite.o \
	${OBJECTDIR}/SOURCE/FUNCTION/TDT.o \
	${OBJECTDIR}/SOURCE/FUNCTION/UNIT_FUNC/TimeUint.o \
	${OBJECTDIR}/SOURCE/FUNCTION/USB.o \
	${OBJECTDIR}/SOURCE/FUNCTION/VWT.o \
	${OBJECTDIR}/SOURCE/FUNCTION/XML.o \
	${OBJECTDIR}/SOURCE/KEY/ProcessTmk.o \
	${OBJECTDIR}/SOURCE/KEY/deslib.o \
	${OBJECTDIR}/SOURCE/NEXSYSESC/NexEscTableFn.o \
	${OBJECTDIR}/SOURCE/NEXSYSESC/NexEscTablePack.o \
	${OBJECTDIR}/SOURCE/NEXSYSESC/NexsysEsc.o \
	${OBJECTDIR}/SOURCE/NEXSYSESC/NexsysEscIso.o \
	${OBJECTDIR}/SOURCE/PRINT/Print.o \
	${OBJECTDIR}/SOURCE/TMS/EDCTmsAnalyseFunc.o \
	${OBJECTDIR}/SOURCE/TMS/EDCTmsFileProc.o \
	${OBJECTDIR}/SOURCE/TMS/EDCTmsFlow.o \
	${OBJECTDIR}/SOURCE/TMS/EDCTmsFtpFunc.o \
	${OBJECTDIR}/SOURCE/TMS/EDCTmsIsoFunc.o \
	${OBJECTDIR}/SOURCE/TMS/EDCTmsPrintFunc.o \
	${OBJECTDIR}/SOURCE/TMS/EDCTmsScheduleFunc.o \
	${OBJECTDIR}/SOURCE/TMS/EDCTmsUnitFunc.o \
	${OBJECTDIR}/SOURCE/TMS/TMSTABLE/EDCtmsFTPFLT.o \
	${OBJECTDIR}/SOURCE/TMS/TMSTABLE/TmsCPT.o \
	${OBJECTDIR}/SOURCE/TMS/TMSTABLE/TmsFLT.o \
	${OBJECTDIR}/SOURCE/TMS/TMSTABLE/TmsFTP.o \
	${OBJECTDIR}/SOURCE/TMS/TMSTABLE/TmsSCT.o \
	${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsCFGFFunc.o \
	${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsCPTFunc.o \
	${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsFTPFLTFunc.o \
	${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsHDPTFunc.o \
	${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsHDTFunc.o \
	${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsMVTFunc.o \
	${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsTDTFunc.o \
	${OBJECTDIR}/SVCSRC/SvcAccum.o \
	${OBJECTDIR}/SVCSRC/SvcCommand.o \
	${OBJECTDIR}/SVCSRC/SvcFunc.o \
	${OBJECTDIR}/SVCSRC/SvcIos.o \
	${OBJECTDIR}/SVCSRC/SvcMenu.o \
	${OBJECTDIR}/SVCSRC/SvcPackTagFunc.o \
	${OBJECTDIR}/SVCSRC/SvcPrint.o \
	${OBJECTDIR}/SVCSRC/SvcSrc.o \
	${OBJECTDIR}/SVCSRC/SvcTable.o \
	${OBJECTDIR}/SortUse.o \
	${OBJECTDIR}/appmain.o


# C Compiler Flags
CFLAGS="-I${SDKV3INC}" -fsigned-char -Wundef -Wstrict-prototypes -Wno-trigraphs -Wimplicit -Wformat 

# CC Compiler Flags
CCFLAGS="-I${SDKV3INC}" -fsigned-char -Wundef -Wstrict-prototypes -Wno-trigraphs -Wimplicit -Wformat 
CXXFLAGS="-I${SDKV3INC}" -fsigned-char -Wundef -Wstrict-prototypes -Wno-trigraphs -Wimplicit -Wformat 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lcaaep -lcabarcode -lcaclentry -lcaclmdl -lcaclvw -lcacqp -lcaddp -lcaemvl2 -lcaemvl2ap -lcaethernet -lcafont -lcafs -lcagsm -lcaifh -lcajct -lcakms -lcalcd -lcamms -lcamodem -lcampp -lcapemenc -lcapmodem -lcaprt -lcartc -lcasqlite -lcatls -lcauart -lcauldpm -lcausbh -lcavap -lcavpw -lcaxml -lcrypto -lctosapi -lcurl -lfreetype -lgmp -liw -lssl -lxml2 -lz -lbmp -lcaqrcode -lcasignature -liconv -lv3_libepadso -lv5scfgexpress -lcacpb -lcarmr -lasound -lv3app_ipassmicropayment -lctp_ecchal -lctp_util test/ECC_PS_Castles_V3_M_MART_v814R01_test.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk dist/V3_Slave_Fubon/NEXSYS.exe

dist/V3_Slave_Fubon/NEXSYS.exe: test/ECC_PS_Castles_V3_M_MART_v814R01_test.a

dist/V3_Slave_Fubon/NEXSYS.exe: ${OBJECTFILES}
	${MKDIR} -p dist/V3_Slave_Fubon
	arm-brcm-linux-gnueabi-g++ -o dist/V3_Slave_Fubon/NEXSYS ${OBJECTFILES} ${LDLIBSOPTIONS} -L . "-L${SDKV3LIB}" "-L${SDKV3LIBN}" 

${OBJECTDIR}/CMAS/CMASAPIISO.o: CMAS/CMASAPIISO.c
	${MKDIR} -p ${OBJECTDIR}/CMAS
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/CMAS/CMASAPIISO.o CMAS/CMASAPIISO.c

${OBJECTDIR}/CMAS/CMASFunction.o: CMAS/CMASFunction.c
	${MKDIR} -p ${OBJECTDIR}/CMAS
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/CMAS/CMASFunction.o CMAS/CMASFunction.c

${OBJECTDIR}/CMAS/CMASMenu.o: CMAS/CMASMenu.c
	${MKDIR} -p ${OBJECTDIR}/CMAS
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/CMAS/CMASMenu.o CMAS/CMASMenu.c

${OBJECTDIR}/CMAS/CMASprtByBuffer.o: CMAS/CMASprtByBuffer.c
	${MKDIR} -p ${OBJECTDIR}/CMAS
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/CMAS/CMASprtByBuffer.o CMAS/CMASprtByBuffer.c

${OBJECTDIR}/CMAS/CMASsrc.o: CMAS/CMASsrc.c
	${MKDIR} -p ${OBJECTDIR}/CMAS
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/CMAS/CMASsrc.o CMAS/CMASsrc.c

${OBJECTDIR}/CTLS/CTLS.o: CTLS/CTLS.c
	${MKDIR} -p ${OBJECTDIR}/CTLS
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/CTLS/CTLS.o CTLS/CTLS.c

${OBJECTDIR}/ECC/ECC.o: ECC/ECC.c
	${MKDIR} -p ${OBJECTDIR}/ECC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ECC.o ECC/ECC.c

${OBJECTDIR}/ECC/ICER/APDU.o: ECC/ICER/APDU.c
	${MKDIR} -p ${OBJECTDIR}/ECC/ICER
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ICER/APDU.o ECC/ICER/APDU.c

${OBJECTDIR}/ECC/ICER/BLC.o: ECC/ICER/BLC.c
	${MKDIR} -p ${OBJECTDIR}/ECC/ICER
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ICER/BLC.o ECC/ICER/BLC.c

${OBJECTDIR}/ECC/ICER/BankData.o: ECC/ICER/BankData.c
	${MKDIR} -p ${OBJECTDIR}/ECC/ICER
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ICER/BankData.o ECC/ICER/BankData.c

${OBJECTDIR}/ECC/ICER/CMAS_APDU.o: ECC/ICER/CMAS_APDU.c
	${MKDIR} -p ${OBJECTDIR}/ECC/ICER
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ICER/CMAS_APDU.o ECC/ICER/CMAS_APDU.c

${OBJECTDIR}/ECC/ICER/CMAS_TM.o: ECC/ICER/CMAS_TM.c
	${MKDIR} -p ${OBJECTDIR}/ECC/ICER
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ICER/CMAS_TM.o ECC/ICER/CMAS_TM.c

${OBJECTDIR}/ECC/ICER/CallAPI.o: ECC/ICER/CallAPI.c
	${MKDIR} -p ${OBJECTDIR}/ECC/ICER
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ICER/CallAPI.o ECC/ICER/CallAPI.c

${OBJECTDIR}/ECC/ICER/Com.o: ECC/ICER/Com.c
	${MKDIR} -p ${OBJECTDIR}/ECC/ICER
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ICER/Com.o ECC/ICER/Com.c

${OBJECTDIR}/ECC/ICER/Function.o: ECC/ICER/Function.c
	${MKDIR} -p ${OBJECTDIR}/ECC/ICER
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ICER/Function.o ECC/ICER/Function.c

${OBJECTDIR}/ECC/ICER/ICERAPI.o: ECC/ICER/ICERAPI.c
	${MKDIR} -p ${OBJECTDIR}/ECC/ICER
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ICER/ICERAPI.o ECC/ICER/ICERAPI.c

${OBJECTDIR}/ECC/ICER/ICERAes.o: ECC/ICER/ICERAes.c
	${MKDIR} -p ${OBJECTDIR}/ECC/ICER
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ICER/ICERAes.o ECC/ICER/ICERAes.c

${OBJECTDIR}/ECC/ICER/ICERLib.o: ECC/ICER/ICERLib.c
	${MKDIR} -p ${OBJECTDIR}/ECC/ICER
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ICER/ICERLib.o ECC/ICER/ICERLib.c

${OBJECTDIR}/ECC/ICER/SIS2.o: ECC/ICER/SIS2.c
	${MKDIR} -p ${OBJECTDIR}/ECC/ICER
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ICER/SIS2.o ECC/ICER/SIS2.c

${OBJECTDIR}/ECC/ICER/SIS2_2.o: ECC/ICER/SIS2_2.c
	${MKDIR} -p ${OBJECTDIR}/ECC/ICER
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ICER/SIS2_2.o ECC/ICER/SIS2_2.c

${OBJECTDIR}/ECC/ICER/SSL.o: ECC/ICER/SSL.c
	${MKDIR} -p ${OBJECTDIR}/ECC/ICER
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ICER/SSL.o ECC/ICER/SSL.c

${OBJECTDIR}/ECC/ICER/TCPIP.o: ECC/ICER/TCPIP.c
	${MKDIR} -p ${OBJECTDIR}/ECC/ICER
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ICER/TCPIP.o ECC/ICER/TCPIP.c

${OBJECTDIR}/ECC/ICER/TM.o: ECC/ICER/TM.c
	${MKDIR} -p ${OBJECTDIR}/ECC/ICER
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ICER/TM.o ECC/ICER/TM.c

${OBJECTDIR}/ECC/ICER/XMLFunc.o: ECC/ICER/XMLFunc.c
	${MKDIR} -p ${OBJECTDIR}/ECC/ICER
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/ECC/ICER/XMLFunc.o ECC/ICER/XMLFunc.c

${OBJECTDIR}/EMVSRC/EMVsrc.o: EMVSRC/EMVsrc.c
	${MKDIR} -p ${OBJECTDIR}/EMVSRC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/EMVSRC/EMVsrc.o EMVSRC/EMVsrc.c

${OBJECTDIR}/EMVSRC/EMVxml.o: EMVSRC/EMVxml.c
	${MKDIR} -p ${OBJECTDIR}/EMVSRC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/EMVSRC/EMVxml.o EMVSRC/EMVxml.c

${OBJECTDIR}/FUBON/FUBONCostoEcr.o: FUBON/FUBONCostoEcr.c
	${MKDIR} -p ${OBJECTDIR}/FUBON
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/FUBON/FUBONCostoEcr.o FUBON/FUBONCostoEcr.c

${OBJECTDIR}/FUBON/FUBONEcr.o: FUBON/FUBONEcr.c
	${MKDIR} -p ${OBJECTDIR}/FUBON
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/FUBON/FUBONEcr.o FUBON/FUBONEcr.c

${OBJECTDIR}/FUBON/FUBONTablePack.o: FUBON/FUBONTablePack.c
	${MKDIR} -p ${OBJECTDIR}/FUBON
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/FUBON/FUBONTablePack.o FUBON/FUBONTablePack.c

${OBJECTDIR}/FUBON/FUBONencryptFunc.o: FUBON/FUBONencryptFunc.c
	${MKDIR} -p ${OBJECTDIR}/FUBON
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/FUBON/FUBONencryptFunc.o FUBON/FUBONencryptFunc.c

${OBJECTDIR}/FUBON/FUBONfunc.o: FUBON/FUBONfunc.c
	${MKDIR} -p ${OBJECTDIR}/FUBON
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/FUBON/FUBONfunc.o FUBON/FUBONfunc.c

${OBJECTDIR}/FUBON/FUBONiso.o: FUBON/FUBONiso.c
	${MKDIR} -p ${OBJECTDIR}/FUBON
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/FUBON/FUBONiso.o FUBON/FUBONiso.c

${OBJECTDIR}/FUBON/FUBONtsam.o: FUBON/FUBONtsam.c
	${MKDIR} -p ${OBJECTDIR}/FUBON
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/FUBON/FUBONtsam.o FUBON/FUBONtsam.c

${OBJECTDIR}/JSON/cJSON.o: JSON/cJSON.c
	${MKDIR} -p ${OBJECTDIR}/JSON
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/JSON/cJSON.o JSON/cJSON.c

${OBJECTDIR}/NCCC/NCCCTicketSrc.o: NCCC/NCCCTicketSrc.c
	${MKDIR} -p ${OBJECTDIR}/NCCC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/NCCC/NCCCTicketSrc.o NCCC/NCCCTicketSrc.c

${OBJECTDIR}/NCCC/NCCCtSAM.o: NCCC/NCCCtSAM.c
	${MKDIR} -p ${OBJECTDIR}/NCCC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/NCCC/NCCCtSAM.o NCCC/NCCCtSAM.c

${OBJECTDIR}/SOURCE/COMM/Bluetooth.o: SOURCE/COMM/Bluetooth.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/COMM
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/COMM/Bluetooth.o SOURCE/COMM/Bluetooth.c

${OBJECTDIR}/SOURCE/COMM/Comm.o: SOURCE/COMM/Comm.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/COMM
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/COMM/Comm.o SOURCE/COMM/Comm.c

${OBJECTDIR}/SOURCE/COMM/Ethernet.o: SOURCE/COMM/Ethernet.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/COMM
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/COMM/Ethernet.o SOURCE/COMM/Ethernet.c

${OBJECTDIR}/SOURCE/COMM/Ftps.o: SOURCE/COMM/Ftps.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/COMM
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/COMM/Ftps.o SOURCE/COMM/Ftps.c

${OBJECTDIR}/SOURCE/COMM/GPRS.o: SOURCE/COMM/GPRS.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/COMM
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/COMM/GPRS.o SOURCE/COMM/GPRS.c

${OBJECTDIR}/SOURCE/COMM/GSM.o: SOURCE/COMM/GSM.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/COMM
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/COMM/GSM.o SOURCE/COMM/GSM.c

${OBJECTDIR}/SOURCE/COMM/Modem.o: SOURCE/COMM/Modem.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/COMM
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/COMM/Modem.o SOURCE/COMM/Modem.c

${OBJECTDIR}/SOURCE/COMM/TLS.o: SOURCE/COMM/TLS.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/COMM
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/COMM/TLS.o SOURCE/COMM/TLS.c

${OBJECTDIR}/SOURCE/COMM/WiFi.o: SOURCE/COMM/WiFi.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/COMM
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/COMM/WiFi.o SOURCE/COMM/WiFi.c

${OBJECTDIR}/SOURCE/DISPLAY/DisTouch.o: SOURCE/DISPLAY/DisTouch.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/DISPLAY
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/DISPLAY/DisTouch.o SOURCE/DISPLAY/DisTouch.c

${OBJECTDIR}/SOURCE/DISPLAY/Display.o: SOURCE/DISPLAY/Display.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/DISPLAY
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/DISPLAY/Display.o SOURCE/DISPLAY/Display.c

${OBJECTDIR}/SOURCE/EVENT/CustomerMenu.o: SOURCE/EVENT/CustomerMenu.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/EVENT
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/EVENT/CustomerMenu.o SOURCE/EVENT/CustomerMenu.c

${OBJECTDIR}/SOURCE/EVENT/Event.o: SOURCE/EVENT/Event.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/EVENT
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/EVENT/Event.o SOURCE/EVENT/Event.c

${OBJECTDIR}/SOURCE/EVENT/EventDispFunc.o: SOURCE/EVENT/EventDispFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/EVENT
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/EVENT/EventDispFunc.o SOURCE/EVENT/EventDispFunc.c

${OBJECTDIR}/SOURCE/EVENT/Flow.o: SOURCE/EVENT/Flow.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/EVENT
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/EVENT/Flow.o SOURCE/EVENT/Flow.c

${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer0Menu.o: SOURCE/EVENT/MENUUI/Layer0Menu.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/EVENT/MENUUI
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer0Menu.o SOURCE/EVENT/MENUUI/Layer0Menu.c

${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer1Menu.o: SOURCE/EVENT/MENUUI/Layer1Menu.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/EVENT/MENUUI
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer1Menu.o SOURCE/EVENT/MENUUI/Layer1Menu.c

${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer2Menu.o: SOURCE/EVENT/MENUUI/Layer2Menu.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/EVENT/MENUUI
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer2Menu.o SOURCE/EVENT/MENUUI/Layer2Menu.c

${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer3Menu.o: SOURCE/EVENT/MENUUI/Layer3Menu.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/EVENT/MENUUI
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer3Menu.o SOURCE/EVENT/MENUUI/Layer3Menu.c

${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer4Menu.o: SOURCE/EVENT/MENUUI/Layer4Menu.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/EVENT/MENUUI
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer4Menu.o SOURCE/EVENT/MENUUI/Layer4Menu.c

${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer5Menu.o: SOURCE/EVENT/MENUUI/Layer5Menu.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/EVENT/MENUUI
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer5Menu.o SOURCE/EVENT/MENUUI/Layer5Menu.c

${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer6Menu.o: SOURCE/EVENT/MENUUI/Layer6Menu.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/EVENT/MENUUI
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/EVENT/MENUUI/Layer6Menu.o SOURCE/EVENT/MENUUI/Layer6Menu.c

${OBJECTDIR}/SOURCE/EVENT/Menu.o: SOURCE/EVENT/Menu.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/EVENT
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/EVENT/Menu.o SOURCE/EVENT/Menu.c

${OBJECTDIR}/SOURCE/FUNCTION/APDU.o: SOURCE/FUNCTION/APDU.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/APDU.o SOURCE/FUNCTION/APDU.c

${OBJECTDIR}/SOURCE/FUNCTION/ASMC.o: SOURCE/FUNCTION/ASMC.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/ASMC.o SOURCE/FUNCTION/ASMC.c

${OBJECTDIR}/SOURCE/FUNCTION/AccountFunction/DispBillInfo.o: SOURCE/FUNCTION/AccountFunction/DispBillInfo.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/AccountFunction
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/AccountFunction/DispBillInfo.o SOURCE/FUNCTION/AccountFunction/DispBillInfo.c

${OBJECTDIR}/SOURCE/FUNCTION/AccountFunction/PrintBillInfo.o: SOURCE/FUNCTION/AccountFunction/PrintBillInfo.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/AccountFunction
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/AccountFunction/PrintBillInfo.o SOURCE/FUNCTION/AccountFunction/PrintBillInfo.c

${OBJECTDIR}/SOURCE/FUNCTION/Accum.o: SOURCE/FUNCTION/Accum.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/Accum.o SOURCE/FUNCTION/Accum.c

${OBJECTDIR}/SOURCE/FUNCTION/BaseUSB.o: SOURCE/FUNCTION/BaseUSB.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/BaseUSB.o SOURCE/FUNCTION/BaseUSB.c

${OBJECTDIR}/SOURCE/FUNCTION/Batch.o: SOURCE/FUNCTION/Batch.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/Batch.o SOURCE/FUNCTION/Batch.c

${OBJECTDIR}/SOURCE/FUNCTION/CARD_FUNC/CardFunction.o: SOURCE/FUNCTION/CARD_FUNC/CardFunction.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/CARD_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/CARD_FUNC/CardFunction.o SOURCE/FUNCTION/CARD_FUNC/CardFunction.c

${OBJECTDIR}/SOURCE/FUNCTION/CDT.o: SOURCE/FUNCTION/CDT.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/CDT.o SOURCE/FUNCTION/CDT.c

${OBJECTDIR}/SOURCE/FUNCTION/CFGT.o: SOURCE/FUNCTION/CFGT.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/CFGT.o SOURCE/FUNCTION/CFGT.c

${OBJECTDIR}/SOURCE/FUNCTION/COSTCO_FUNC/Costco.o: SOURCE/FUNCTION/COSTCO_FUNC/Costco.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/COSTCO_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/COSTCO_FUNC/Costco.o SOURCE/FUNCTION/COSTCO_FUNC/Costco.c

${OBJECTDIR}/SOURCE/FUNCTION/CPT.o: SOURCE/FUNCTION/CPT.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/CPT.o SOURCE/FUNCTION/CPT.c

${OBJECTDIR}/SOURCE/FUNCTION/CPT_Backup.o: SOURCE/FUNCTION/CPT_Backup.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/CPT_Backup.o SOURCE/FUNCTION/CPT_Backup.c

${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC/CreditCheckFunc.o: SOURCE/FUNCTION/CREDIT_FUNC/CreditCheckFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC/CreditCheckFunc.o SOURCE/FUNCTION/CREDIT_FUNC/CreditCheckFunc.c

${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC/CreditFlowDispFunc.o: SOURCE/FUNCTION/CREDIT_FUNC/CreditFlowDispFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC/CreditFlowDispFunc.o SOURCE/FUNCTION/CREDIT_FUNC/CreditFlowDispFunc.c

${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC/CreditPrintFunc.o: SOURCE/FUNCTION/CREDIT_FUNC/CreditPrintFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC/CreditPrintFunc.o SOURCE/FUNCTION/CREDIT_FUNC/CreditPrintFunc.c

${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC/CreditProcessDataFunc.o: SOURCE/FUNCTION/CREDIT_FUNC/CreditProcessDataFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC/CreditProcessDataFunc.o SOURCE/FUNCTION/CREDIT_FUNC/CreditProcessDataFunc.c

${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC/Creditfunc.o: SOURCE/FUNCTION/CREDIT_FUNC/Creditfunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/CREDIT_FUNC/Creditfunc.o SOURCE/FUNCTION/CREDIT_FUNC/Creditfunc.c

${OBJECTDIR}/SOURCE/FUNCTION/CUP_FUNC/CupCheckFunc.o: SOURCE/FUNCTION/CUP_FUNC/CupCheckFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/CUP_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/CUP_FUNC/CupCheckFunc.o SOURCE/FUNCTION/CUP_FUNC/CupCheckFunc.c

${OBJECTDIR}/SOURCE/FUNCTION/CUP_FUNC/CupFlowDispFunc.o: SOURCE/FUNCTION/CUP_FUNC/CupFlowDispFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/CUP_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/CUP_FUNC/CupFlowDispFunc.o SOURCE/FUNCTION/CUP_FUNC/CupFlowDispFunc.c

${OBJECTDIR}/SOURCE/FUNCTION/CUP_FUNC/CupProcessDataFunc.o: SOURCE/FUNCTION/CUP_FUNC/CupProcessDataFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/CUP_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/CUP_FUNC/CupProcessDataFunc.o SOURCE/FUNCTION/CUP_FUNC/CupProcessDataFunc.c

${OBJECTDIR}/SOURCE/FUNCTION/Card.o: SOURCE/FUNCTION/Card.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/Card.o SOURCE/FUNCTION/Card.c

${OBJECTDIR}/SOURCE/FUNCTION/ECCDT.o: SOURCE/FUNCTION/ECCDT.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/ECCDT.o SOURCE/FUNCTION/ECCDT.c

${OBJECTDIR}/SOURCE/FUNCTION/ECR_FUNC/ECR.o: SOURCE/FUNCTION/ECR_FUNC/ECR.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/ECR_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/ECR_FUNC/ECR.o SOURCE/FUNCTION/ECR_FUNC/ECR.c

${OBJECTDIR}/SOURCE/FUNCTION/ECR_FUNC/EcrPackFunc.o: SOURCE/FUNCTION/ECR_FUNC/EcrPackFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/ECR_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/ECR_FUNC/EcrPackFunc.o SOURCE/FUNCTION/ECR_FUNC/EcrPackFunc.c

${OBJECTDIR}/SOURCE/FUNCTION/ECR_FUNC/EcrUnPackFunc.o: SOURCE/FUNCTION/ECR_FUNC/EcrUnPackFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/ECR_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/ECR_FUNC/EcrUnPackFunc.o SOURCE/FUNCTION/ECR_FUNC/EcrUnPackFunc.c

${OBJECTDIR}/SOURCE/FUNCTION/ECR_FUNC/RS232.o: SOURCE/FUNCTION/ECR_FUNC/RS232.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/ECR_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/ECR_FUNC/RS232.o SOURCE/FUNCTION/ECR_FUNC/RS232.c

${OBJECTDIR}/SOURCE/FUNCTION/EDC.o: SOURCE/FUNCTION/EDC.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/EDC.o SOURCE/FUNCTION/EDC.c

${OBJECTDIR}/SOURCE/FUNCTION/EDC_Para_Table_Func.o: SOURCE/FUNCTION/EDC_Para_Table_Func.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/EDC_Para_Table_Func.o SOURCE/FUNCTION/EDC_Para_Table_Func.c

${OBJECTDIR}/SOURCE/FUNCTION/EST.o: SOURCE/FUNCTION/EST.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/EST.o SOURCE/FUNCTION/EST.c

${OBJECTDIR}/SOURCE/FUNCTION/FILE_FUNC/FIleLogFunc.o: SOURCE/FUNCTION/FILE_FUNC/FIleLogFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/FILE_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/FILE_FUNC/FIleLogFunc.o SOURCE/FUNCTION/FILE_FUNC/FIleLogFunc.c

${OBJECTDIR}/SOURCE/FUNCTION/FILE_FUNC/File.o: SOURCE/FUNCTION/FILE_FUNC/File.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/FILE_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/FILE_FUNC/File.o SOURCE/FUNCTION/FILE_FUNC/File.c

${OBJECTDIR}/SOURCE/FUNCTION/FuncTable.o: SOURCE/FUNCTION/FuncTable.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/FuncTable.o SOURCE/FUNCTION/FuncTable.c

${OBJECTDIR}/SOURCE/FUNCTION/Function.o: SOURCE/FUNCTION/Function.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/Function.o SOURCE/FUNCTION/Function.c

${OBJECTDIR}/SOURCE/FUNCTION/HDPT.o: SOURCE/FUNCTION/HDPT.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/HDPT.o SOURCE/FUNCTION/HDPT.c

${OBJECTDIR}/SOURCE/FUNCTION/HDT.o: SOURCE/FUNCTION/HDT.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/HDT.o SOURCE/FUNCTION/HDT.c

${OBJECTDIR}/SOURCE/FUNCTION/INSTALL_FUNC/InstCheckFunc.o: SOURCE/FUNCTION/INSTALL_FUNC/InstCheckFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/INSTALL_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/INSTALL_FUNC/InstCheckFunc.o SOURCE/FUNCTION/INSTALL_FUNC/InstCheckFunc.c

${OBJECTDIR}/SOURCE/FUNCTION/INSTALL_FUNC/InstProcessDataFunc.o: SOURCE/FUNCTION/INSTALL_FUNC/InstProcessDataFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/INSTALL_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/INSTALL_FUNC/InstProcessDataFunc.o SOURCE/FUNCTION/INSTALL_FUNC/InstProcessDataFunc.c

${OBJECTDIR}/SOURCE/FUNCTION/IPASSDT.o: SOURCE/FUNCTION/IPASSDT.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/IPASSDT.o SOURCE/FUNCTION/IPASSDT.c

${OBJECTDIR}/SOURCE/FUNCTION/KMS.o: SOURCE/FUNCTION/KMS.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/KMS.o SOURCE/FUNCTION/KMS.c

${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC/ExDevicePackData.o: SOURCE/FUNCTION/MULTI_FUNC/ExDevicePackData.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC/ExDevicePackData.o SOURCE/FUNCTION/MULTI_FUNC/ExDevicePackData.c

${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC/ExDeviceUnPackData.o: SOURCE/FUNCTION/MULTI_FUNC/ExDeviceUnPackData.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC/ExDeviceUnPackData.o SOURCE/FUNCTION/MULTI_FUNC/ExDeviceUnPackData.c

${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC/JsonMultiHostFunc.o: SOURCE/FUNCTION/MULTI_FUNC/JsonMultiHostFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC/JsonMultiHostFunc.o SOURCE/FUNCTION/MULTI_FUNC/JsonMultiHostFunc.c

${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC/MultiFunc.o: SOURCE/FUNCTION/MULTI_FUNC/MultiFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC/MultiFunc.o SOURCE/FUNCTION/MULTI_FUNC/MultiFunc.c

${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC/MultiHostFunc.o: SOURCE/FUNCTION/MULTI_FUNC/MultiHostFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/MULTI_FUNC/MultiHostFunc.o SOURCE/FUNCTION/MULTI_FUNC/MultiHostFunc.c

${OBJECTDIR}/SOURCE/FUNCTION/MVT.o: SOURCE/FUNCTION/MVT.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/MVT.o SOURCE/FUNCTION/MVT.c

${OBJECTDIR}/SOURCE/FUNCTION/PCD.o: SOURCE/FUNCTION/PCD.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/PCD.o SOURCE/FUNCTION/PCD.c

${OBJECTDIR}/SOURCE/FUNCTION/PIT.o: SOURCE/FUNCTION/PIT.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/PIT.o SOURCE/FUNCTION/PIT.c

${OBJECTDIR}/SOURCE/FUNCTION/PWD.o: SOURCE/FUNCTION/PWD.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/PWD.o SOURCE/FUNCTION/PWD.c

${OBJECTDIR}/SOURCE/FUNCTION/PowerManagement.o: SOURCE/FUNCTION/PowerManagement.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/PowerManagement.o SOURCE/FUNCTION/PowerManagement.c

${OBJECTDIR}/SOURCE/FUNCTION/QAT.o: SOURCE/FUNCTION/QAT.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/QAT.o SOURCE/FUNCTION/QAT.c

${OBJECTDIR}/SOURCE/FUNCTION/REDEEM_FUNC/RedeemCheckFunc.o: SOURCE/FUNCTION/REDEEM_FUNC/RedeemCheckFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/REDEEM_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/REDEEM_FUNC/RedeemCheckFunc.o SOURCE/FUNCTION/REDEEM_FUNC/RedeemCheckFunc.c

${OBJECTDIR}/SOURCE/FUNCTION/REDEEM_FUNC/RedeemProcessDataFunc.o: SOURCE/FUNCTION/REDEEM_FUNC/RedeemProcessDataFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/REDEEM_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/REDEEM_FUNC/RedeemProcessDataFunc.o SOURCE/FUNCTION/REDEEM_FUNC/RedeemProcessDataFunc.c

${OBJECTDIR}/SOURCE/FUNCTION/SCDT.o: SOURCE/FUNCTION/SCDT.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/SCDT.o SOURCE/FUNCTION/SCDT.c

${OBJECTDIR}/SOURCE/FUNCTION/SKM.o: SOURCE/FUNCTION/SKM.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/SKM.o SOURCE/FUNCTION/SKM.c

${OBJECTDIR}/SOURCE/FUNCTION/Signpad.o: SOURCE/FUNCTION/Signpad.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/Signpad.o SOURCE/FUNCTION/Signpad.c

${OBJECTDIR}/SOURCE/FUNCTION/Sqlite.o: SOURCE/FUNCTION/Sqlite.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/Sqlite.o SOURCE/FUNCTION/Sqlite.c

${OBJECTDIR}/SOURCE/FUNCTION/TDT.o: SOURCE/FUNCTION/TDT.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/TDT.o SOURCE/FUNCTION/TDT.c

${OBJECTDIR}/SOURCE/FUNCTION/UNIT_FUNC/TimeUint.o: SOURCE/FUNCTION/UNIT_FUNC/TimeUint.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION/UNIT_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/UNIT_FUNC/TimeUint.o SOURCE/FUNCTION/UNIT_FUNC/TimeUint.c

${OBJECTDIR}/SOURCE/FUNCTION/USB.o: SOURCE/FUNCTION/USB.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/USB.o SOURCE/FUNCTION/USB.c

${OBJECTDIR}/SOURCE/FUNCTION/VWT.o: SOURCE/FUNCTION/VWT.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/VWT.o SOURCE/FUNCTION/VWT.c

${OBJECTDIR}/SOURCE/FUNCTION/XML.o: SOURCE/FUNCTION/XML.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/FUNCTION
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/FUNCTION/XML.o SOURCE/FUNCTION/XML.c

${OBJECTDIR}/SOURCE/KEY/ProcessTmk.o: SOURCE/KEY/ProcessTmk.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/KEY
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/KEY/ProcessTmk.o SOURCE/KEY/ProcessTmk.c

${OBJECTDIR}/SOURCE/KEY/deslib.o: SOURCE/KEY/deslib.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/KEY
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/KEY/deslib.o SOURCE/KEY/deslib.c

${OBJECTDIR}/SOURCE/NEXSYSESC/NexEscTableFn.o: SOURCE/NEXSYSESC/NexEscTableFn.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/NEXSYSESC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/NEXSYSESC/NexEscTableFn.o SOURCE/NEXSYSESC/NexEscTableFn.c

${OBJECTDIR}/SOURCE/NEXSYSESC/NexEscTablePack.o: SOURCE/NEXSYSESC/NexEscTablePack.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/NEXSYSESC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/NEXSYSESC/NexEscTablePack.o SOURCE/NEXSYSESC/NexEscTablePack.c

${OBJECTDIR}/SOURCE/NEXSYSESC/NexsysEsc.o: SOURCE/NEXSYSESC/NexsysEsc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/NEXSYSESC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/NEXSYSESC/NexsysEsc.o SOURCE/NEXSYSESC/NexsysEsc.c

${OBJECTDIR}/SOURCE/NEXSYSESC/NexsysEscIso.o: SOURCE/NEXSYSESC/NexsysEscIso.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/NEXSYSESC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/NEXSYSESC/NexsysEscIso.o SOURCE/NEXSYSESC/NexsysEscIso.c

${OBJECTDIR}/SOURCE/PRINT/Print.o: SOURCE/PRINT/Print.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/PRINT
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/PRINT/Print.o SOURCE/PRINT/Print.c

${OBJECTDIR}/SOURCE/TMS/EDCTmsAnalyseFunc.o: SOURCE/TMS/EDCTmsAnalyseFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/EDCTmsAnalyseFunc.o SOURCE/TMS/EDCTmsAnalyseFunc.c

${OBJECTDIR}/SOURCE/TMS/EDCTmsFileProc.o: SOURCE/TMS/EDCTmsFileProc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/EDCTmsFileProc.o SOURCE/TMS/EDCTmsFileProc.c

${OBJECTDIR}/SOURCE/TMS/EDCTmsFlow.o: SOURCE/TMS/EDCTmsFlow.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/EDCTmsFlow.o SOURCE/TMS/EDCTmsFlow.c

${OBJECTDIR}/SOURCE/TMS/EDCTmsFtpFunc.o: SOURCE/TMS/EDCTmsFtpFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/EDCTmsFtpFunc.o SOURCE/TMS/EDCTmsFtpFunc.c

${OBJECTDIR}/SOURCE/TMS/EDCTmsIsoFunc.o: SOURCE/TMS/EDCTmsIsoFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/EDCTmsIsoFunc.o SOURCE/TMS/EDCTmsIsoFunc.c

${OBJECTDIR}/SOURCE/TMS/EDCTmsPrintFunc.o: SOURCE/TMS/EDCTmsPrintFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/EDCTmsPrintFunc.o SOURCE/TMS/EDCTmsPrintFunc.c

${OBJECTDIR}/SOURCE/TMS/EDCTmsScheduleFunc.o: SOURCE/TMS/EDCTmsScheduleFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/EDCTmsScheduleFunc.o SOURCE/TMS/EDCTmsScheduleFunc.c

${OBJECTDIR}/SOURCE/TMS/EDCTmsUnitFunc.o: SOURCE/TMS/EDCTmsUnitFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/EDCTmsUnitFunc.o SOURCE/TMS/EDCTmsUnitFunc.c

${OBJECTDIR}/SOURCE/TMS/TMSTABLE/EDCtmsFTPFLT.o: SOURCE/TMS/TMSTABLE/EDCtmsFTPFLT.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS/TMSTABLE
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/TMSTABLE/EDCtmsFTPFLT.o SOURCE/TMS/TMSTABLE/EDCtmsFTPFLT.c

${OBJECTDIR}/SOURCE/TMS/TMSTABLE/TmsCPT.o: SOURCE/TMS/TMSTABLE/TmsCPT.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS/TMSTABLE
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/TMSTABLE/TmsCPT.o SOURCE/TMS/TMSTABLE/TmsCPT.c

${OBJECTDIR}/SOURCE/TMS/TMSTABLE/TmsFLT.o: SOURCE/TMS/TMSTABLE/TmsFLT.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS/TMSTABLE
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/TMSTABLE/TmsFLT.o SOURCE/TMS/TMSTABLE/TmsFLT.c

${OBJECTDIR}/SOURCE/TMS/TMSTABLE/TmsFTP.o: SOURCE/TMS/TMSTABLE/TmsFTP.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS/TMSTABLE
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/TMSTABLE/TmsFTP.o SOURCE/TMS/TMSTABLE/TmsFTP.c

${OBJECTDIR}/SOURCE/TMS/TMSTABLE/TmsSCT.o: SOURCE/TMS/TMSTABLE/TmsSCT.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS/TMSTABLE
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/TMSTABLE/TmsSCT.o SOURCE/TMS/TMSTABLE/TmsSCT.c

${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsCFGFFunc.o: SOURCE/TMS/TMSTABLE_FUNC/EDCTmsCFGFFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsCFGFFunc.o SOURCE/TMS/TMSTABLE_FUNC/EDCTmsCFGFFunc.c

${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsCPTFunc.o: SOURCE/TMS/TMSTABLE_FUNC/EDCTmsCPTFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsCPTFunc.o SOURCE/TMS/TMSTABLE_FUNC/EDCTmsCPTFunc.c

${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsFTPFLTFunc.o: SOURCE/TMS/TMSTABLE_FUNC/EDCTmsFTPFLTFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsFTPFLTFunc.o SOURCE/TMS/TMSTABLE_FUNC/EDCTmsFTPFLTFunc.c

${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsHDPTFunc.o: SOURCE/TMS/TMSTABLE_FUNC/EDCTmsHDPTFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsHDPTFunc.o SOURCE/TMS/TMSTABLE_FUNC/EDCTmsHDPTFunc.c

${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsHDTFunc.o: SOURCE/TMS/TMSTABLE_FUNC/EDCTmsHDTFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsHDTFunc.o SOURCE/TMS/TMSTABLE_FUNC/EDCTmsHDTFunc.c

${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsMVTFunc.o: SOURCE/TMS/TMSTABLE_FUNC/EDCTmsMVTFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsMVTFunc.o SOURCE/TMS/TMSTABLE_FUNC/EDCTmsMVTFunc.c

${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsTDTFunc.o: SOURCE/TMS/TMSTABLE_FUNC/EDCTmsTDTFunc.c
	${MKDIR} -p ${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SOURCE/TMS/TMSTABLE_FUNC/EDCTmsTDTFunc.o SOURCE/TMS/TMSTABLE_FUNC/EDCTmsTDTFunc.c

${OBJECTDIR}/SVCSRC/SvcAccum.o: SVCSRC/SvcAccum.c
	${MKDIR} -p ${OBJECTDIR}/SVCSRC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SVCSRC/SvcAccum.o SVCSRC/SvcAccum.c

${OBJECTDIR}/SVCSRC/SvcCommand.o: SVCSRC/SvcCommand.c
	${MKDIR} -p ${OBJECTDIR}/SVCSRC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SVCSRC/SvcCommand.o SVCSRC/SvcCommand.c

${OBJECTDIR}/SVCSRC/SvcFunc.o: SVCSRC/SvcFunc.c
	${MKDIR} -p ${OBJECTDIR}/SVCSRC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SVCSRC/SvcFunc.o SVCSRC/SvcFunc.c

${OBJECTDIR}/SVCSRC/SvcIos.o: SVCSRC/SvcIos.c
	${MKDIR} -p ${OBJECTDIR}/SVCSRC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SVCSRC/SvcIos.o SVCSRC/SvcIos.c

${OBJECTDIR}/SVCSRC/SvcMenu.o: SVCSRC/SvcMenu.c
	${MKDIR} -p ${OBJECTDIR}/SVCSRC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SVCSRC/SvcMenu.o SVCSRC/SvcMenu.c

${OBJECTDIR}/SVCSRC/SvcPackTagFunc.o: SVCSRC/SvcPackTagFunc.c
	${MKDIR} -p ${OBJECTDIR}/SVCSRC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SVCSRC/SvcPackTagFunc.o SVCSRC/SvcPackTagFunc.c

${OBJECTDIR}/SVCSRC/SvcPrint.o: SVCSRC/SvcPrint.c
	${MKDIR} -p ${OBJECTDIR}/SVCSRC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SVCSRC/SvcPrint.o SVCSRC/SvcPrint.c

${OBJECTDIR}/SVCSRC/SvcSrc.o: SVCSRC/SvcSrc.c
	${MKDIR} -p ${OBJECTDIR}/SVCSRC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SVCSRC/SvcSrc.o SVCSRC/SvcSrc.c

${OBJECTDIR}/SVCSRC/SvcTable.o: SVCSRC/SvcTable.c
	${MKDIR} -p ${OBJECTDIR}/SVCSRC
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SVCSRC/SvcTable.o SVCSRC/SvcTable.c

${OBJECTDIR}/SortUse.o: SortUse.c
	${MKDIR} -p ${OBJECTDIR}
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/SortUse.o SortUse.c

${OBJECTDIR}/appmain.o: appmain.c
	${MKDIR} -p ${OBJECTDIR}
	$(COMPILE.c) -g -Wall -DLIB_DEFINE -DRDEVN\ =\ NE_NEXSYS_V3 -DREADER_MANUFACTURERS=LINUX_API -D_FUBON_MAIN_HOST_ -D_MACHINE_TYPE_\ =\ _CASTLE_TYPE_V3C_ -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA3000/include -I/cygdrive/C/cygwin -I../../../../cygwin64 -I../eric_chang/CodeBlocks/MinGW -o ${OBJECTDIR}/appmain.o appmain.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
