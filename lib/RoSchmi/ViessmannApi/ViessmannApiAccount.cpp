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
ViessmannApiAccount::ViessmannApiAccount(String clientId, String accessToken, String apiIot, String apiUser, String apiToken, bool useHttps )
{
    ChangeAccountParams(clientId, accessToken, apiIot, apiUser, apiToken, useHttps);   
}

void ViessmannApiAccount::ChangeAccountParams(String clientId, String accessToken, String apiIot, String apiUser, String apiToken, bool useHttps)
{
    ClientId = clientId;
    ClientId = (clientId.length() <= MAX_CLIENT_ID_LENGTH) ? clientId : clientId.substring(0, MAX_CLIENT_ID_LENGTH);
    AccessToken = accessToken;
    ApiIotUrl = apiIot;
    ApiUserUrl = apiIot;
    ApiTokenUrl = apiToken;
    
    const char * insert = (char *)useHttps ? "s" : "";
    int baseLength = apiIot.length() > apiUser.length() ? apiIot.length() : apiUser.length();
    char strData[baseLength + 30];
    
    sprintf(strData, "http%s://%s", insert, apiIot.c_str());
    UriEndPointIot = String(strData);
    
    sprintf(strData, "http%s://%s", insert, apiUser.c_str());
    UriEndPointUser = String(strData);
    
    sprintf(strData, "http%s://%s", insert, apiToken.c_str());
    UriEndPointToken = String(strData);
}

void ViessmannApiAccount::RenewAccessToken(String accessToken)
{
    AccessToken = accessToken;
}



/**
 * destructor
 */
ViessmannApiAccount::~ViessmannApiAccount()
{}