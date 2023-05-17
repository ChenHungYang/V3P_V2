/* 
 * File:   CMASMenu.h
 * Author: Hachi
 *
 * Created on 2020年1月3日, 下午 3:48
 */

#ifndef CMASMENU_H
#define CMASMENU_H

#ifdef __cplusplus
extern "C"
{
#endif

int inCMAS_MENU_ICON(EventMenuItem *srEventMenuItem);
int inCMAS_MENU_SALE(EventMenuItem *srEventMenuItem);
int inCMAS_MENU_REFUND(EventMenuItem *srEventMenuItem);
int inCMAS_MENU_ADD_VALUE(EventMenuItem *srEventMenuItem);
int inCMAS_MENU_VOID_ADD_VALUE(EventMenuItem *srEventMenuItem);
int inCMAS_MENU_SETTLE(EventMenuItem *srEventMenuItem);
int inCMAS_MENU_QUERY(EventMenuItem *srEventMenuItem);
int inCMAS_MENU_Check_Enable(void);

#ifdef __cplusplus
}
#endif

#endif /* CMASMENU_H */






