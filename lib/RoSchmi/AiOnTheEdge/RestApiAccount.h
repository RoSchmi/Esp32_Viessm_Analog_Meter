#include <Arduino.h>

#ifndef _RESTAPIACCOUNT_H_
#define _RESTAPIACCOUNT_H_

#define MAX_ACCOUNTNAME_LENGTH 50
#define ACCOUNT_KEY_LENGTH     88

class RestApiAccount
{
public:
    RestApiAccount();
    RestApiAccount(String accountName, String accountKey, String hostName, bool useHttps);
    ~RestApiAccount();

    void ChangeAccountParams(String accountName, String accountKey, String hostName, bool useHttps);
    
    String AccountName;
    String AccountKey;
    //String UriEndPointBlob;
    //String UriEndPointQueue;
    String UriEndPointJson;
    //String HostNameBlob;
    //String HostNameQueue;
    String HostName;
};


#endif  // _RESTAPIACCOUNT_H_