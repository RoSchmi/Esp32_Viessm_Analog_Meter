#ifndef _CONFIG_SECRET_H
#define _CONFIG_SECRET_H

// Rename config_secret_template.h into config_secret.h to activate the content

// Wifi (may be changed in WiFi-Manager)
#define IOT_CONFIG_WIFI_SSID            "YourSSID"
#define IOT_CONFIG_WIFI_PASSWORD        "YourWiFiPassword"


// Azure Storage Credentials (may be changed in the Config Portal Page)
#define AZURE_CONFIG_ACCOUNT_NAME       "YourAzureStorageAccountName"
#define AZURE_CONFIG_ACCOUNT_KEY        "3M+ssz2Ws6....YourStorageAccountKey.....BOjg+1r7ZA=="

// The following three tokens need not be set to valid values here
// The Viessmann Client Id and the Viessmann Refresh Token 
// can be set in the Config Portal Page 
// However they can be set here for initial setting and debugging purposes 
#define VIESSMANN_CLIENT_ID "888888........888888888888"
#define VIESSMANN_REFRESH_TOKEN "004280382eadbe....d4953ffd19d27c"
#define VIESSMANN_ACCESS_TOKEN "very..long..token"

#endif // _CONFIG_SECRET_H