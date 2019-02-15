#ifndef STR_PARSE_H
#define STR_PARSE_H

#define STR_PARAM_MAX_NUM 32
#define STR_CMD_MAX_PARAM_NUM 6

u8 StrParamParse(char *pParamStr,char **pName,char **pVal);
u8 StrCmdParse(const char *pCmdStr,char **pRet,char *pBuf,bool CmdToLower);

#endif

