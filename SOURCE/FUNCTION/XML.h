/* 
 * File:   XML.h
 * Author: user
 *
 * Created on 2018年3月27日, 下午 2:10
 */

int inXML_ParseFile(char* szFileName, xmlDocPtr* srDoc);
int inXML_SaveFile(char *szFilename, xmlDocPtr* srDoc, char *szEncoding, int inFormat);
int inXML_Get_RootElement(xmlDocPtr* srDoc, xmlNodePtr* srCur);
int inXML_FreeDOC(xmlDocPtr* srdoc);
int inXML_CleanParser(void);
int inXML_MemoryDump(void);
int inXML_End(xmlDocPtr* srdoc);
int inXML_ReadFile(char* szFileName, xmlDocPtr* srDoc);

