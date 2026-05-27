#include "commun_manager/commun_manager.h"
#include <stdlib.h>
#include "init/init.h"




CommMethod_t SelectCommMethod(void)
{
    CommMethod_t result = COMM_SATELLITE;   // 默认走卫星（失败时不变）

    /* 1. 确保 4G 模块上电 */
    FourG_PowerOnAndWait();

    /* 2. 查询信号强度 */
    int rssi = FourG_GetSignalQuality();
    if (rssi < 0 || rssi < 20 || rssi == 99) {
        osDelay(2000);
        rssi = FourG_GetSignalQuality();
        if (rssi < 20 || rssi == 99) {
            goto cleanup;
        }
    }

    /* 3. 检查 SIM 卡是否就绪 */
    if (!FourG_CheckSIMReady()) goto cleanup;

    /* 4. 网络注册 */
    if (!FourG_IsNetworkRegistered()) {
        if (!FourG_AttachGPRS()) goto cleanup;
        osDelay(2000);
        if (!FourG_IsNetworkRegistered()) goto cleanup;
    }


    /* 5. 定义 APN */
    if (!FourG_DefineAPN("CMNET")) goto cleanup;

    /* 6. 激活 PDP */
    if (!FourG_ActivatePDP(1)) goto cleanup;

    /* 7. 获取 IP */
    char ip[16];
    if (!FourG_GetIPAddress(ip, sizeof(ip))) goto cleanup;

    /* 8. 建立 TCP 连接 */
    FourG_CloseSocket(0);
    char server_ip[20]="";
    sprintf(server_ip, "%d.%d.%d.%d", g_server_ip[0], g_server_ip[1], g_server_ip[2], g_server_ip[3]);
    if (!FourG_OpenTCPSocket(0, server_ip, g_server_port)) goto cleanup;

    /* 全部通过，改用 4G */
    result = COMM_4G;

    cleanup:
        if (result == COMM_SATELLITE) {
            FourG_PowerOffAndWait();   // 失败时彻底断电，省电并复位模块状态
        }
    return result;
}