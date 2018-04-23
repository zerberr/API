
namespace java API
namespace cpp api

typedef string Currency;
typedef string Address;
typedef i64 Time;

struct Amount 
{
  1: required i32 integral = 0;
  2: required i64 fraction = 0;
}

typedef map<Currency, Amount> Balance;

//
// Transactions
//

typedef string TransactionHash
typedef string TransactionId
typedef string TransactionInnerId

struct Transaction
{
    1: TransactionHash hash
    2: TransactionInnerId innerId
    3: Address source
    4: Address target
    5: Amount amount
    6: Currency currency
}

typedef list<Transaction> Transactions
typedef list<TransactionId> TransactionIds

//
//  Pools
//

typedef binary PoolHash
typedef i64 PoolNumber

struct Pool
{
    1: PoolHash hash
    2: PoolHash prevHash
    3: Time time
    4: i32 transactionsCount
    5: PoolNumber poolNumber
}

typedef list<Pool> Pools

//
//  Stats
//

typedef i32 Count

struct PeriodStats
{
    1: Time periodDuration
    2: Count poolsCount
    3: Count transactionsCount
    4: Balance balancePerCurrency
}

typedef list<PeriodStats> StatsPerPeriod

//
// API responses
//

struct APIResponse
{
    1: i8 code
    2: string message
}

// BalanceGet

struct BalanceGetResult
{
    1: APIResponse status
    2: Amount amount
}

// TransactionGet

struct TransactionGetResult
{
    1: APIResponse status
    2: bool found
    3: Transaction transaction
}

// TransactionsGet

struct TransactionsGetResult
{
    1: APIResponse status
    2: bool result
    3: Transactions transactions
}

struct TransactionFlowResult
{
    1: APIResponse status
}

// PoolListGet

struct PoolListGetResult
{
    1: APIResponse status
    2: bool result
    3: Pools pools
}

// PoolGet

struct PoolGetResult
{
    1: APIResponse status
    2: Pool pool
    3: Transactions transactions
}

// StatsGet

struct StatsGetResult
{
    1: APIResponse status
    2: StatsPerPeriod stats
}

// NodesCountGet

struct NodesCountGetResult
{
    1: APIResponse status
    2: Count count
}

service API 
{
    BalanceGetResult BalanceGet(1:Address address, 2:Currency currency = 'cs')
   
    TransactionGetResult TransactionGet(1:TransactionId transactionId)
    TransactionsGetResult TransactionsGet(1:Address address, 2:i64 offset, 3:i64 limit)
    TransactionFlowResult TransactionFlow(1:Transaction transaction)
    
    PoolListGetResult PoolListGet(1:i64 offset, 2:i64 limit)
    PoolGetResult PoolGet(1:PoolHash hash)
    
    StatsGetResult StatsGet()

    NodesCountGetResult NodesCountGet()
}
