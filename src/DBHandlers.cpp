#include "DBHandlers.h"

// csdb
#include <csdb/wallet.h>
#include <csdb/pool.h>
#include <csdb/transaction.h>
#include <csdb/currency.h>
#include <csdb/amount.h>
#include <csdb/address.h>

#include "csconnector/csconnector.h"
#include <algorithm>
#include <tuple>
#include <cstring>

using namespace api;

csdb::Storage DbHandlers::s_storage{};

void DbHandlers::init()
{
    const bool isStorageOpen = s_storage.open();

    std::string message;
    if (isStorageOpen)
    {
        message = "Storage is opened normal";

        using namespace csconnector;

        registerHandler<Commands::BalanceGet>(BalanceGet);
        registerHandler<Commands::TransactionGet>(TransactionGet);
        registerHandler<Commands::TransactionsGet>(TransactionsGet);
        registerHandler<Commands::PoolInfoGet>(PoolInfoGet);
        registerHandler<Commands::PoolTransactionsGet>(PoolTransactionsGet);
        registerHandler<Commands::PoolListGet>(PoolListGet);
    }
    else
    {
        message += "Storage is not opedened. Reason: " + s_storage.last_error_message();
    }

    std::cout << message << std::endl;
}

void DbHandlers::deinit()
{
    using namespace csconnector;

    unregisterHandler<Commands::BalanceGet>();
    unregisterHandler<Commands::TransactionGet>();
    unregisterHandler<Commands::TransactionsGet>();
    unregisterHandler<Commands::PoolInfoGet>();
    unregisterHandler<Commands::PoolTransactionsGet>();
    unregisterHandler<Commands::PoolListGet>();
}

void DbHandlers::BalanceGet(BalanceGetResult& _return, const Address& address, const Currency& currency)
{
    const csdb::Wallet wallet = s_storage.wallet(csdb::Address::from_hex(address));
//    const csdb::Wallet wallet = s_storage.wallet(csdb::Address::from_string(address)); // раскомментировать когда добавится
    const csdb::Amount amount = wallet.amount(csdb::Currency(currency));
    _return.amount.integral   = amount.integral();
    _return.amount.fraction   = amount.fraction();
}

void DbHandlers::TransactionGet(TransactionGetResult& _return, const TransactionId& transactionId)
{
    const csdb::TransactionID tempTransactionId; // =  csdb::TransactionID::from_string(transactionId); // раскомментить когда появится

    const csdb::Transaction& transaction = s_storage.transaction(tempTransactionId);

    _return.found = transaction.is_valid();


    const csdb::Currency& currency = transaction.currency();
    _return.transaction.currency   = currency.to_string();

    const std::map<csdb::Address, csdb::Amount>& destinations = transaction.destinations();
    _return.transaction.amount                                = calculateAmount(destinations);

    const csdb::TransactionID& id = transaction.id();
    _return.transaction.innerId = id.to_string();

    const csdb::Address& payerAddress = transaction.payer();
    _return.transaction.source        = payerAddress.to_string();

    _return.transaction.destinations  = extractAddresses(transaction.destinations());
}

void DbHandlers::TransactionsGet(TransactionsGetResult& _return, const Address& address, const int64_t offset, const int64_t limit)
{
    const std::vector<csdb::Transaction>& transactions = s_storage.transactions(csdb::Address::from_hex(address)); // правильно так создавать csdb::Address?

    // раскомментировать когда появится метод Address::from_string()
    //    const std::vector<csdb::Transaction>& transactions = s_storage.transactions(csdb::Address::from_string(address));

    _return.transactions = convertTransactions(transactions);
}

void DbHandlers::PoolListGet(PoolListGetResult &_return, const int64_t offset, const int64_t limit)
{
    csdb::PoolHash offsetedPoolHash;
    for (int64_t i = 0; i < offset; ++i)
    {
        const csdb::PoolHash& tmp = s_storage.last_hash();
        offsetedPoolHash = s_storage.pool(tmp).previous_hash();
    }

    for (int64_t i = 0; i < limit; ++i)
    {
        api::Pool apiPool;
        const csdb::Pool& csdbPool = s_storage.pool(offsetedPoolHash);

        apiPool.hash              = offsetedPoolHash.to_string();
        apiPool.poolNumber        = csdbPool.sequence();
        apiPool.prevHash          = csdbPool.previous_hash().to_string();
//      apiPool.time              = // раскомментировать когда появится дополнительное поле
        apiPool.transactionsCount = csdbPool.transactions_count();

        _return.pools.push_back(apiPool);
    }
}

void DbHandlers::PoolInfoGet(PoolInfoGetResult& _return, const PoolHash& hash)
{
//    const csdb::TransactionID::from_string(hash);
    const csdb::PoolHash poolHash;

    const csdb::Pool pool = s_storage.pool(poolHash);

    _return.isFound = pool.is_valid(); // логически НЕ верно! pool.isValid проверяет есть ли в пуле транзакции! А не найден он или нет
    // Пул может быть без транзакций. Как понять найден он или нет?

    _return.pool.hash = poolHash.to_string(); // Выяснить! Какой хеш мы возвращаем (мы же его на вход передаём)

    _return.pool.poolNumber = static_cast<int64_t>(pool.sequence());

    const csdb::PoolHash& previousHash = pool.previous_hash();
    _return.pool.prevHash  = previousHash.to_string();


//    _return.pool.time   // Добавить когда появится дополнительное поле в csdb::Pool

    _return.pool.transactionsCount = pool.transactions_count();
}

void DbHandlers::PoolTransactionsGet(PoolTransactionsGetResult& _return, const PoolHash& poolHashString, const int64_t offset, const int64_t limit)
{
    const csdb::PoolHash poolHash;

    csdb::Pool pool = s_storage.pool(poolHash);

    const std::vector<csdb::Transaction>& transactions = pool.transactions();

    _return.transactions = convertTransactions(transactions);
}

Amount DbHandlers::calculateAmount(const std::map<csdb::Address, csdb::Amount>& destinations)
{
    api::Amount result;

    csdb::Amount totalAmount;
    for (const std::pair<csdb::Address, csdb::Amount>& it : destinations)
    {
        totalAmount += it.second;
    }

    result.integral = totalAmount.integral();
    result.fraction = totalAmount.fraction();

    return result;
}

Transactions DbHandlers::convertTransactions(const std::vector<csdb::Transaction>& transactions)
{
    api::Transactions result;

    for (const csdb::Transaction& csdbTransaction : transactions)
    {
        api::Transaction apiTransaction;

        apiTransaction.amount   = calculateAmount(csdbTransaction.destinations());  // цикл в цикле

        const csdb::Currency& currency = csdbTransaction.currency();
        apiTransaction.currency        = currency.to_string();

        const csdb::TransactionID& id = csdbTransaction.id();
        apiTransaction.innerId        = id.to_string(); // inner - внутренний, по логике вещей это верное присваивание. Уточнить!

        const csdb::Address& payerAddress = csdbTransaction.payer();
        apiTransaction.source             = payerAddress.to_string();

        apiTransaction.destinations = extractAddresses(csdbTransaction.destinations());

        result.push_back(apiTransaction);
    }

    return result;
}

Addresses DbHandlers::extractAddresses(const std::map<csdb::Address, csdb::Amount>& destinations)
{
    Addresses result;
    for (const std::pair<csdb::Address, csdb::Amount>& it : destinations)
    {
        result.push_back(it.first.to_string());
    }
    return result;
}
