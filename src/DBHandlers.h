#ifndef DB_HANDLERS_H
#define DB_HANDLERS_H

#include "API.h"

namespace db_handlers
{
	void init();
	void deinit();

	void BalanceGet(api::BalanceGetResult& _return, const api::Address& address, const api::Currency& currency);
	void TransactionGet(api::TransactionGetResult& _return, const api::TransactionId& transactionId);
	void TransactionsGet(api::TransactionsGetResult& _return, const api::Address& address, const int64_t offset, const int64_t limit);
	void PoolListGet(api::PoolListGetResult& _return, const int64_t offset, const int64_t limit);
	void PoolGet(api::PoolGetResult& _return, const api::PoolHash& hash);
}

#endif // DB_HANDLERS_H