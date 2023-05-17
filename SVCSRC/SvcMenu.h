/* 
 * File:   SvcMenu.h
 * Author: Sam chang
 *
 * Created on 2022年12月26日, 下午 5:16
 */

#ifndef SVCMENU_H
#define	SVCMENU_H

#ifdef	__cplusplus
extern "C" {
#endif

int inMENU_SVC_ACTIVECARD_AUTORELOAD(EventMenuItem *srEventMenuItem);
int inMENU_SVC_INQUIRY(EventMenuItem *srEventMenuItem);
int inMENU_SVC_RE_ACTIVECARD(EventMenuItem *srEventMenuItem);
int inMENU_SVC_REFUND(EventMenuItem *srEventMenuItem);
int inMENU_SVC_BACKCARD(EventMenuItem *srEventMenuItem);
int inMENU_SVC_RELOAD(EventMenuItem *srEventMenuItem);

#ifdef	__cplusplus
}
#endif

#endif	/* SVCMENU_H */

