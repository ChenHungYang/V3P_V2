
#ifndef __LAYER_1_MENU_H__
#define __LAYER_1_MENU_H__

int inMENU01_FunctionNewUi(EventMenuItem *srEventMenuItem);
int inMENU01_MenuKeyInAndGetAmount(EventMenuItem *srEventMenuItem);
int inMENU01_Swipe(EventMenuItem *srEventMenuItem);
int inMENU01_ICC(EventMenuItem *srEventMenuItem);
int inMENU01_MultiFunctionSlave(EventMenuItem *srEventMenuItem);
int inMENU01_ECR(EventMenuItem *srEventMenuItem);
int inMENU01_EscIdleUpload(EventMenuItem *srEventMenuItem);
int inMENU01_DccSchedule(EventMenuItem *srEventMenuItem);
int inMENU01_TmsDccSchedule(EventMenuItem *srEventMenuItem);
int inMENU01_TmsSchduleInquire(EventMenuItem *srEventMenuItem);
int inMENU01_TmsSchduleDownload(EventMenuItem *srEventMenuItem);
int inMENU01_TmsProcessEffective(EventMenuItem *srEventMenuItem);
int inMENU01_EdcBooting(EventMenuItem *srEventMenuItem);
int inMENU01_PowerManagement(EventMenuItem *srEventMenuItem);

#endif
