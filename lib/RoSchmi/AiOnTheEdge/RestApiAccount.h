#include <Arduino.h>

#ifndef _RESTAPIACCOUNT_H_
#define _RESTAPIACCOUNT_H_

#define MAX_ACCOUNTNAME_LENGTH 50
#define ACCOUNT_KEY_LENGTH     88

class RestApiAccount
{
public:
    RestApiAccount();
    RestApiAccount(const String accountName, const String accountKey, const String hostName, const bool useHttps, const bool useCaCert);
    
    void ChangeAccountParams(const String accountName, const String accountKey, const String hostName, const bool useHttps, const bool useCaCert);
    
    String AccountName;
    String AccountKey;   
    String UriEndPointJson;
    String BaseUrl;   
    String HostName;

    bool UseHttps;
    bool UseCaCert;
};
#endif  // _RESTAPIACCOUNT_H_