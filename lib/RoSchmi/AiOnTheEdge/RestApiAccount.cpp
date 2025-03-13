#include <RestApiAccount.h>

/**
 * constructor
 */
RestApiAccount::RestApiAccount(String accountName, String accountKey, String hostName, bool useHttps )
{
    AccountName = (accountName.length() <= MAX_ACCOUNTNAME_LENGTH) ? accountName : accountName.substring(0, MAX_ACCOUNTNAME_LENGTH);
    AccountKey = accountKey;
    HostName = (hostName.length() <= MAX_ACCOUNTNAME_LENGTH) ? hostName : hostName.substring(0, MAX_ACCOUNTNAME_LENGTH);
    char strData[accountName.length() + 30];
    const char * insert = (char *)useHttps ? "s" : "";

    sprintf(strData, "http%s://%s/%s", insert, HostName.c_str(), "json");
    UriEndPointJson = String(strData);  
}

void RestApiAccount::ChangeAccountParams(String accountName, String accountKey, String hostName, bool useHttps)
{
    AccountName = (accountName.length() <= MAX_ACCOUNTNAME_LENGTH) ? accountName : accountName.substring(0, MAX_ACCOUNTNAME_LENGTH);
    AccountKey = accountKey;
    char strData[accountName.length() + 30];
    const char * insert = (char *)useHttps ? "s" : "";
    sprintf(strData, "http%s://%s/%s/", insert, HostName.c_str(), "json");
    UriEndPointJson = String(strData);  
}



/**
 * destructor
 */
RestApiAccount::~RestApiAccount()
{}