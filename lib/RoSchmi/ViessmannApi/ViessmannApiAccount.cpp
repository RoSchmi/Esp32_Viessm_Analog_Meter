#include <ViessmannApiAccount.h>

/**
 * @brief 
 * 
 * @param clientId 
 * @param accessToken 
 * @param apiIot 
 * @param apiUser 
 * @param useHttps 
 * @return * constructor 
 */
ViessmannApiAccount::ViessmannApiAccount(const String clientId, const String accessToken, const String apiIot, const String apiUser, const String apiToken, const bool useHttps, const bool useCaCert)
{
    ChangeAccountParams(clientId, accessToken, apiIot, apiUser, apiToken, useHttps, useCaCert);   
}

void ViessmannApiAccount::ChangeAccountParams(const String clientId, const String accessToken, const String apiIot, const String apiUser, const String apiToken, const bool useHttps, const bool useCaCert)
{
    ClientId = clientId;
    ClientId = (clientId.length() <= MAX_CLIENT_ID_LENGTH) ? clientId : clientId.substring(0, MAX_CLIENT_ID_LENGTH);
    AccessToken = accessToken;
    ApiIotUrl = apiIot;
    ApiUserUrl = apiIot;
    ApiTokenUrl = apiToken;

    UseHttps = useHttps;
    UseCaCert = useCaCert;
    
    const char * insert = (char *)useHttps ? "s" : "";
    int baseLength = apiIot.length() > apiUser.length() ? apiIot.length() : apiUser.length();
    // RoSchmi baseLength must be constant
    //char strData[baseLength + 30];
    char strData[60 + 30];
    
    sprintf(strData, "http%s://%s", insert, apiIot.c_str());
    UriEndPointIot = String(strData);
    
    sprintf(strData, "http%s://%s", insert, apiUser.c_str());
    UriEndPointUser = String(strData);
    
    sprintf(strData, "http%s://%s", insert, apiToken.c_str());
    UriEndPointToken = String(strData);
}

void ViessmannApiAccount::RenewAccessToken(const String accessToken)
{
    AccessToken = accessToken;
}
