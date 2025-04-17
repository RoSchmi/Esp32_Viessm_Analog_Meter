
#include <Arduino.h>

#ifndef _CLOUDSTORAGEACCOUNT_H_
#define _CLOUDSTORAGEACCOUNT_H_

#define MAX_ACCOUNTNAME_LENGTH 50
#define ACCOUNT_KEY_LENGTH     88

class CloudStorageAccount
{
public:
    CloudStorageAccount();
    CloudStorageAccount(const String accountName, const String accountKey, const bool useHttps, const bool useCaCert);
    ~CloudStorageAccount();

    void ChangeAccountParams(const String accountName, const String accountKey, const bool useHttps, const bool useCaCert);
    
    String AccountName;
    String AccountKey;
    //String UriEndPointBlob;
    //String UriEndPointQueue;
    String UriEndPointTable;
    //String HostNameBlob;
    //String HostNameQueue;
    String HostNameTable;

    bool UseHttps;
    bool UseCaCert;
};


#endif  // _CLOUDSTORAGEACCOUNT_H_