/*
*********************************************************************************************************
*
*                                      User console task process
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_task_console.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#include <includes.h>
#include <user_core.h>
#include <user_task_console.h>
#include <user_os_func.h>
#include <user_ep_manage.h>
#include "tfs.h"
#include <eem.h>

typedef enum tagFuncLevle
{
    FUNC_LEVEL_E_UNUSED = 0,
    FUNC_LEVEL_E_SYS    = 1,
    FUNC_LEVEL_E_USER   = 2,
}FUNC_LEVEL_E;

typedef void (*pfnCON_FUNCTION)(void);

typedef struct tagConFuncDesc
{
    u8              level;
    char            cmd[16];
    pfnCON_FUNCTION proc;
    u8              hlpStr[32];
}CON_FUNC_DESC_S;

/*----------------macros declare here---------------------*/
#define MAX_CONSOLE_OSQ_SIZE 32

extern EP_INFO_S g_stMyEpInfo;

/*---------------- global vars declare here---------------------*/
OS_EVENT            *g_QSemConsoleMsgRecv = NULL;


/*----------------local global vars declare here---------------------*/
static OS_STK     g_UserTaskConsoleSTK[DEFAULT_USER_CONSOLE_STK_SIZE];
static void       *g_ConsoleOSQ[MAX_CONSOLE_OSQ_SIZE];

static char         gCmd[16]        = {0};
static char         gCmdPara1[16]   = {0};
static char         gCmdPara2[32]   = {0};
static char         gCmdPara3[8]    = {0};

static u8           gOutBuf[256]    = {0};


/*----------------local global funcs declare here---------------------*/
static void Console_TaskRecvProc(void *p_arg);
static void Console_Init(void);
static void Console_EventRecvLoop(void);

static void OnDebug(void);

static void OnHelp(void);
static void OnReboot(void);
static void OnPrint(void);
static void OnSet(void);

#define MAX_CONSOLE_FUNCTION   5

static CON_FUNC_DESC_S gCONSOLE_FUNCTIONS[MAX_CONSOLE_FUNCTION] = 
{
    {FUNC_LEVEL_E_SYS, "help", OnHelp, "显示帮助!"},
    {FUNC_LEVEL_E_SYS, "reboot", OnReboot, "设备重启!"},  
    {FUNC_LEVEL_E_SYS, "dbg", OnDebug, "调试选项!"},
    {FUNC_LEVEL_E_SYS, "print", OnPrint, "打印信息!"},    
    {FUNC_LEVEL_E_SYS, "set", OnSet, "设备参数设置!"}, 
};


void Console_Init(void)
{
    g_QSemConsoleMsgRecv = OSQCreate(&g_ConsoleOSQ[0], MAX_CONSOLE_OSQ_SIZE);
    if (NULL == g_QSemConsoleMsgRecv)
    {
        UCORE_DEBUG_PRINT("Create g_QSemConsoleMsgRecv failed.\r\n");
    }

    UCORE_DEBUG_PRINT("User task Console init finished.\r\n");
}

u8 UConsole_Start(void)
{
    u8          ucResult    = UCORE_ERR_COMMON_FAILED;

    UCORE_DEBUG_PRINT("starting user task Console...\r\n");

    /* call init first */
    Console_Init();    

    ucResult = OSTaskCreate(Console_TaskRecvProc, (void *)0, (OS_STK *)&g_UserTaskConsoleSTK[DEFAULT_USER_CONSOLE_STK_SIZE - 1], DEFAULT_USER_TASK_CONSOLE_PRIO);
    if (OS_ERR_NONE != ucResult)
    {
        UCORE_DEBUG_PRINT("User task Console create Failed, value:%d.\r\n", ucResult);
        return UCORE_ERR_CREATE_TASK_FAILED;
    }

    return UCORE_ERR_SUCCESS;
}

void Console_TaskRecvProc(void *p_arg)
{
    UCORE_DEBUG_PRINT("User Console task start.\r\n");

    /* enter event loop */
    Console_EventRecvLoop();

    UCORE_DEBUG_PRINT("User Console task exit.\r\n");

    return;
}

void Console_EventRecvLoop(void)
{
    u16             i               = 0;
    u16             usLen           = 0;
    u8              ucPara          = 0;
    u8              ucLastPos       = 0;
    u8              ucParaLen       = 0;
    u8              ucProcessed     = 0;
    INT8U           err             = 2;    
    char            *pcBuff         = NULL;        
    
    while (1)
    {
        /* wait for message in */
        pcBuff = (char *) OSQPend(g_QSemConsoleMsgRecv, 0, &err);
        if ((OS_NO_ERR == err) && (NULL != pcBuff))
        {
            usLen = strlen(pcBuff);

            /* check */
            switch (pcBuff[0])
            {
                case ASCII_LF:
                case ASCII_CR:
                {
                    break;
                }
                default:
                {
                    ucProcessed = 0;
                    
                    /* clear vars */
                    memset(gCmd, 0, 16);
                    memset(gCmdPara1, 0, 16);
                    memset(gCmdPara2, 0, 32);  
                    memset(gCmdPara3, 0, 8);
                    
                    /* here start parse command and paras */
                    for (i=0, ucPara=0, ucLastPos=0; i<usLen; i++)
                    {
                        if ((pcBuff[i] == ' ') || (ASCII_LF == pcBuff[i]))
                        {
                            ucParaLen = ASCII_LF == pcBuff[i] ? i - ucLastPos - 1 : i - ucLastPos;
                            
                            if (ucPara == 0) /* is command */
                            {
                                if (ucParaLen > 8)
                                {
                                    printf("\nCommand must less than 8 words.");
                                    break;
                                }
                                
                                strncpy(gCmd, pcBuff, ucParaLen);
                                
                                ucPara++;
                                ucLastPos = i + 1;

                                //printf("\nCommand:%s", gCmd);
                            }
                            else if(ucPara == 1)
                            {
                                if (ucParaLen > 8)
                                {
                                    printf("\nPara1 must less than 8 words.");
                                    break;
                                }
                                
                                strncpy(gCmdPara1, pcBuff + ucLastPos, ucParaLen);
                                
                                ucPara++;
                                ucLastPos = i + 1;
                                //printf("\nPara1:%s", gCmdPara1);                                
                            }
                            else if(ucPara == 2)
                            {
                                if (ucParaLen >= 32)
                                {
                                    printf("\nPara2 must less than 32 words.");
                                    break;
                                }
                                
                                strncpy(gCmdPara2, pcBuff + ucLastPos, ucParaLen);
                                
                                ucPara++;
                                ucLastPos = i + 1;
                                //printf("\nPara2:%s", gCmdPara2);
                            }
                            else if (ucPara == 3)
                            {
                                if (ucParaLen >= 4)
                                {
                                    printf("\nPara3 must less than 4 words.");
                                    break;
                                }
                                
                                strncpy(gCmdPara3, pcBuff + ucLastPos, ucParaLen);
                                
                                ucPara++;
                                ucLastPos = i + 1;
                                printf("\nPara3:%s", gCmdPara3);                            
                            }
                        }
                    }

                    /* ok, here call functions */
                    for (i=0; i<MAX_CONSOLE_FUNCTION; i++)
                    {
                        if (strncmp(gCONSOLE_FUNCTIONS[i].cmd, gCmd, strlen(gCmd)) == 0)
                        {
                            gCONSOLE_FUNCTIONS[i].proc();
                            
                            ucProcessed = 1;
                            break;
                        }
                    }

                    if (0 == ucProcessed)
                        printf("\nUnknown command name!\n");

                    
                    break;
                }
            }

            /* print prompt string */
            printf("\nCONSOLE>");
                    
            /* clean buff */
            free(pcBuff);
        }
    }
}


static void OnHelp()
{
    u8 i = 0;

    printf("-------------------------------------------------------------------\n");
    printf("|%-9s|         |%s|\n", "command", "describe");    
    printf("-------------------------------------------------------------------\n");    
    for (i=0; i<MAX_CONSOLE_FUNCTION; i++)
    {
        printf("%-9s         %s\n", gCONSOLE_FUNCTIONS[i].cmd, gCONSOLE_FUNCTIONS[i].hlpStr);    
    }
}

static void OnReboot()
{
    SystemReset();
}

static void OnDebug(void)
{
    u8 ucPara1 = 0;
    
    /* check para len */
    if (strlen(gCmdPara1) < 2)
    {
        ucPara1 = '*';
    }
    else
    {
        ucPara1 = gCmdPara1[1];
    }

    switch (ucPara1)
    {
        case 'i':
        {
            if (strlen(gCmdPara2) == 0)
            {
                printf("Set debug switch, but no command input!\r\n");
                return;
            }

            /* check on or off */
            if (strcmp(gCmdPara2, "ON") == 0)
            {
                g_udev_config.enDebug = 1;
            }
            else if (strcmp(gCmdPara2, "OFF") == 0)
            {
                g_udev_config.enDebug = 0;
            }
            else
            {
                printf("Para is not valid, please check ON or OFF? ");
                return;
            }

            /* save to eeprom */
            if (TFS_RESULT_SUCESS != TFS_Write(TFS_CONFIG_ITEM_DEV_CONF, sizeof(UDEV_CONFIG_S), &g_udev_config))
            {
                printf("Set config failed!\r\n");
                return;
            }            

            printf("Set debug info:[%s]\r\n", gCmdPara2);
            
            break;
        }
        case 'g':
        {
            if (strlen(gCmdPara2) == 0)
            {
                printf("Set GPS debug switch, but no command input!\r\n");
                return;
            }

            /* check on or off */
            if (strcmp(gCmdPara2, "ON") == 0)
            {
                g_udev_config.enGpsDebug= 1;
            }
            else if (strcmp(gCmdPara2, "OFF") == 0)
            {
                g_udev_config.enGpsDebug = 0;
            }
            else
            {
                printf("Para is not valid, please check ON or OFF? ");
                return;
            }

            /* save to eeprom */
            if (TFS_RESULT_SUCESS != TFS_Write(TFS_CONFIG_ITEM_DEV_CONF, sizeof(UDEV_CONFIG_S), &g_udev_config))
            {
                printf("Set config failed!\r\n");
                return;
            }            

            printf("Set GPS debug info:[%s]\r\n", gCmdPara2);
            
            break;
        }        
        case 'l':
        {
            if (1 == g_udev_config.enDebug)
            {
                printf("DEBUG:  ON\r\n");
            }
            else
            {
                printf("DEBUG:  OFF\r\n");
            }

            if (1 == g_udev_config.enGpsDebug)
            {
                printf("GPS DEBUG:  ON\r\n");
            }
            else
            {
                printf("GPS DEBUG:  OFF\r\n");
            }
            
            break;
        }
        default:
        {
			printf("dbg command:\n");
			printf("dbg  -i <ON>/<OFF>:  设置调试信息输出开关\n");
            printf("dbg  -g <ON>/<OFF>:  设置GPS调试信息输出开关\n");
            printf("dbg  -l:             查看调试信息输出开关\n");
            break;
        }
    }
}

static void OnPrint(void)
{
    u8 ucPara1 = 0;
    
    /* check para len */
    if (strlen(gCmdPara1) < 2)
    {
        ucPara1 = '*';
    }
    else
    {
        ucPara1 = gCmdPara1[1];
    }

    switch (ucPara1)
    {
        case 'e':
        {
            u16     i           = 0;
            u8      j           = 0;
            u8      k           = 0;
            u8      len         = 0;
            u16     usSize      = 0;
            char    *pstr       = NULL;            
            u16     addr[2]     = {0};            

            len = strlen(gCmdPara2);
            if (len == 0)
            {
                printf("No para input!\r\n");
                return;
            }

            /* make a dot at end */
            gCmdPara2[len] = ':';
            gCmdPara2[len + 1] = 0;

            pstr = gCmdPara2;
            while ((gCmdPara2[i] != 0) && (j < 2))
            {
                gCmdPara2[i] = toupper(gCmdPara2[i]);
                
                if (gCmdPara2[i] == ':')
                {
                    char saddr[6] = {0};
                    u8   addrlen = &gCmdPara2[i] - pstr;

                    if (addrlen > 5)
                    {
                        printf("Address is not valid, formt example: 0:16384\r\n");
                        return;
                    }
                    
                    strncpy(saddr, pstr, addrlen);

                    /* check ip string */
                    for (k=0; saddr[k] != 0; k++)
                    {
                        if ((saddr[k] < 0x30) || (saddr[k] > 0x39))
                        {
                            printf("Address is not decimal!\r\n");
                            
                            return;
                        }
                    }

                    /* ok, here safty asign value */
                    addr[j] = atoi(saddr);
                    
                    j++;                    

                    /* change ptr */
                    pstr = &gCmdPara2[i + 1];
                }

                i++;
            }

            /* ok, now check addr */
            if (addr[0] >= addr[1])
            {
                printf("Begin addr must smaller than end addr.\r\n");
                return;
            }

            /* cal size */
            usSize = addr[1] - addr[0];
            if (usSize > 256)
            {
                printf("Dump buffer size must smaller than 256.\r\n");
                return;
            }            

            /* 读EEPROM, 起始地址 = addr[0]， 数据长度为 addr[1] - addr[0] */
            if (TFS_Dump((u8 *)gOutBuf, addr[0], usSize) > 0)
            {
                printf("读eeprom成功，数据如下：\r\n");
                
                /* 打印eeprom数据 */
                for (i = 0; i < usSize; i++)
                {
                    printf(" %02X", gOutBuf[i]);

                    if ((i & 15) == 15)
                    {
                        printf("\r\n");	/* 每行显示8字节数据 */
                    }		
                }
            }
            
            break;
        }
        case 'p':
        {
            printf("PAN ID:%d\r\n", g_udev_config.PanId);
            
            break;
        }
        case 'd':
        {
            u16             usTotalLen  = 0; 
            EEM_HEADER_S    *pHeader    = NULL;
            u8              *pcBuff     = NULL;          

            /* create header */
        	pHeader = EEM_CreateHeader(0, EEM_COMMAND_GET_EPID, UCORE_ERR_SUCCESS);
        	if (NULL == pHeader)
        	{
        	    UCORE_DEBUG_PRINT("EEM CreateHeader failed.\r\n");
                return;
            }
            
            /* get buff */
        	pcBuff = EEM_GetBuff(pHeader, &usTotalLen);
            if (NULL != pcBuff)
            {
                USART_Send(USART2, usTotalLen, (void *) pcBuff);
            }

            /* clean buffer */
            EEM_Delete((void **) &pHeader);

            /* wait for a while */
            OSTimeDlyHMSM(0, 0, 0, 100);
            
            printf("EP ID:%d\r\n", g_stMyEpInfo.ucEpId);

            break;
        }
        default:
        {
			printf("print command:\n");
			printf("print  -e <DEC>:<DEC>:  查看EEPROM内容, 10进制地址\n");
            printf("print  -p            :  查看设备当前PANDID\n");
            printf("print  -d            :  查看设备当前EPID\n");
            break;
        }
    }
}

static void OnSet(void)
{
    u8 ucPara1 = 0;
    
    /* check para len */
    if (strlen(gCmdPara1) < 2)
    {
        ucPara1 = '*';
    }
    else
    {
        ucPara1 = gCmdPara1[1];
    }

    switch (ucPara1)
    {
        case 'p':
        {
            u8              ucRet       = 0;
            u16             i           = 0;
            u8              len         = 0;
            u16             pandid      = 0;           
            u16             usTotalLen  = 0; 
            EEM_HEADER_S    *pHeader    = NULL;
            u8              *pcBuff     = NULL;

            len = strlen(gCmdPara2);
            if (len == 0)
            {
                printf("No panid input!\r\n");
                return;
            }

            if (len > 5)
            {
                printf("Pandid is not valid, range: [0,16383] or 65535 \r\n");
                return;
            }
            
            /* check ip string */
            for (i=0; gCmdPara2[i] != 0; i++)
            {
                /* check range */
                if ((gCmdPara2[i] < 0x30) || (gCmdPara2[i] > 0x39))
                {
                    printf("Pandid is not decimal!\r\n");
                    
                    return;
                }
            }

            /* ok, here safty asign value */
            pandid = atoi(gCmdPara2);

            /* check range */
            if ((pandid > 16383) && (65535 != pandid))
            {
                printf("Please check panid range: [0,16383] or 65535.\r\n");
                return;
            }            

            /* 校验完成，通知协调器切换PANID */
        	pHeader = EEM_CreateHeader(0, EEM_COMMAND_SET_PANDID, UCORE_ERR_SUCCESS);
        	if (NULL == pHeader)
        	{
        	    UCORE_DEBUG_PRINT("EEM CreateHeader failed.\r\n");
                return;
            }

            /* check force flag */
            if (strlen(gCmdPara3) > 0)
            {
                if (gCmdPara3[1] == 'f')
                {
                    pHeader->ucSPara = 1;
                }
            }            

        	ucRet = EEM_AppendPayload(&pHeader, EEM_PAYLOAD_TYPE_COORD_NWK_ID, sizeof(u16), (void *) &pandid);
            if (UCORE_ERR_SUCCESS != ucRet)
            {
                /* clean buffer */
                EEM_Delete((void **) &pHeader);
                break;
            }

            /* get buff */
        	pcBuff = EEM_GetBuff(pHeader, &usTotalLen);
            if (NULL != pcBuff)
            {
                USART_Send(USART2, usTotalLen, (void *) pcBuff);
            }

            /* clean buffer */
            EEM_Delete((void **) &pHeader);
    
            break;
        }
        case 'd':
        {
            u8              ucRet       = 0;
            u16             i           = 0;
            u8              len         = 0;
            u16             epid        = 0;           
            u16             usTotalLen  = 0; 
            EEM_HEADER_S    *pHeader    = NULL;
            u8              *pcBuff     = NULL;

            len = strlen(gCmdPara2);
            if (len == 0)
            {
                printf("No panid input!\r\n");
                return;
            }

            if (len > 3)
            {
                printf("EP ID is not valid, range: [0,255]\r\n");
                return;
            }
            
            /* check ip string */
            for (i=0; gCmdPara2[i] != 0; i++)
            {
                /* check range */
                if ((gCmdPara2[i] < 0x30) || (gCmdPara2[i] > 0x39))
                {
                    printf("EP ID is not decimal!\r\n");
                    
                    return;
                }
            }

            /* ok, here safty asign value */
            epid = atoi(gCmdPara2);

            /* check range */
            if (epid > 255)
            {
                printf("Please check panid range: [0,255].\r\n");
                return;
            }            

            /* 校验完成，通知2530设置EPID */
        	pHeader = EEM_CreateHeader(0, EEM_COMMAND_SET_EPID, UCORE_ERR_SUCCESS);
        	if (NULL == pHeader)
        	{
        	    UCORE_DEBUG_PRINT("EEM CreateHeader failed.\r\n");
                return;
            }       

        	ucRet = EEM_AppendPayload(&pHeader, EEM_PAYLOAD_TYPE_EP_ID, sizeof(u16), (void *) &epid);
            if (UCORE_ERR_SUCCESS != ucRet)
            {
                /* clean buffer */
                EEM_Delete((void **) &pHeader);
                break;
            }

            /* get buff */
        	pcBuff = EEM_GetBuff(pHeader, &usTotalLen);
            if (NULL != pcBuff)
            {
                USART_Send(USART2, usTotalLen, (void *) pcBuff);
            }

            /* clean buffer */
            EEM_Delete((void **) &pHeader);
    
            break;
        }        
        default:
        {
			printf("set command:\n");
			printf("set  -p <panid>:  设置设备入网PAN ID, 范围:[0,16383]或者65535表示随机!\n");
            printf("set  -d <epid>:   设置设备的终端EP ID, 范围:[0,255]!\n");
            break;
        }
    }
}


