#include <RestApiAccount.h>

/**
 * constructor
 */
RestApiAccount::RestApiAccount(const String accountName, const String accountKey, const String hostName, const bool useHttps, const bool useCaCert )
{
    AccountName = (accountName.length() <= MAX_ACCOUNTNAME_LENGTH) ? accountName : accountName.substring(0, MAX_ACCOUNTNAME_LENGTH);
    AccountKey = accountKey;
    HostName = (hostName.length() <= MAX_ACCOUNTNAME_LENGTH) ? hostName : hostName.substring(0, MAX_ACCOUNTNAME_LENGTH);
    char strData[accountName.length() + 30];
    const char * insert = (char *)useHttps ? "s" : "";
    sprintf(strData, "http%s://%s", insert, HostName.c_str());
    BaseUrl = String(strData);   
    sprintf(strData, "http%s://%s/%s", insert, HostName.c_str(), "json");
    UriEndPointJson = String(strData);  
}

void RestApiAccount::ChangeAccountParams(const String accountName, const String accountKey, const String hostName, bool useHttps, const bool useCaCert)
{
    AccountName = (accountName.length() <= MAX_ACCOUNTNAME_LENGTH) ? accountName : accountName.substring(0, MAX_ACCOUNTNAME_LENGTH);
    AccountKey = accountKey;
    char strData[accountName.length() + 30];
    const char * insert = (char *)useHttps ? "s" : "";
    sprintf(strData, "http%s://%s/%s/", insert, HostName.c_str(), "json");
    UriEndPointJson = String(strData);  
}

