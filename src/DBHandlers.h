#ifndef DB_HANDLERS_H
#define DB_HANDLERS_H

#include "API.h"

#include <csdb/storage.h>
#include <csdb/amount.h>
#include <csdb/address.h>

class DbHandlers
{
public:
    static void init();
    static void deinit();

    static void BalanceGet(api::BalanceGetResult& _return, const api::Address& address, const api::Currency& currency);
    static void TransactionGet(api::TransactionGetResult& _return, const api::TransactionId& transactionId);
    static void TransactionsGet(api::TransactionsGetResult& _return, const api::Address& address, const int64_t offset, const int64_t limit);
    static void PoolListGet(api::PoolListGetResult& _return, const int64_t offset, const int64_t limit);
    static void PoolInfoGet(api::PoolInfoGetResult& _return, const api::PoolHash& hash);
    static void PoolTransactionsGet(api::PoolTransactionsGetResult& _return, const api::PoolHash& poolHashString, const int64_t offset, const int64_t limit);

private:
//    static Transaction ConvertTransaction(const csdb::Transaction& tran);

private:
    static api::Transactions convertTransactions(const std::vector<csdb::Transaction>& transactions);
    static api::Amount calculateAmount(const std::map<csdb::Address, csdb::Amount>& destinations);
    static api::Addresses extractAddresses(const std::map<csdb::Address, csdb::Amount>& destinations);

private:
    static csdb::Storage s_storage;
};


#endif // DB_HANDLERS_H
