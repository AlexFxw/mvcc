/**
 * Forward declare the classes and define the inline functions
 * Include those headers from standard libraries
 */

#ifndef MVCC_FWD_H
#define MVCC_FWD_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include <limits>
#include <memory>
#include <ctime>
#include <chrono>
#include <iostream>
#include <glog/logging.h>


namespace mvcc {


typedef int ValueType;
typedef std::string KeyType;
typedef uint32_t TxnId;
typedef uint64_t TxnStamp;

// Used for initialization in assignment 1
const static TxnId INSERT_NO_ID = std::numeric_limits<TxnId>::max();

inline TxnStamp GetTimeStamp() {
    auto now = std::chrono::high_resolution_clock::now();
    uint64_t nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    return nanos;
}

// TODO: Add abort in assignment 3?
enum class OP {
    READ,
    BEGIN,
    COMMIT,
    SET,
    INSERT,
    INVALID
};

enum class MathOp {
    PLUS,
    MINUS,
    TIMES,
    DIVIDE,
    INVALID
};

struct Operation {
    OP op;
    KeyType key;
    MathOp mathOp; // + - * /
    ValueType value;

    Operation() : op(OP::INVALID), mathOp(MathOp::INVALID) {}

    Operation(OP op, ValueType txn) : op(op), value(txn) {
        // Used for initialize the txn operation
        DCHECK(op == OP::BEGIN || op == OP::COMMIT);
    }

    Operation(OP op, KeyType key, ValueType value = 0, MathOp mOp = MathOp::INVALID) :
            op(op), key(std::move(key)), value(value), mathOp(mOp) {
        if (op == OP::SET)
            DCHECK(mathOp != MathOp::INVALID);
    }
};

struct Txn {
    std::vector<Operation> operations;
    bool committed;
    TxnId txnId;

    Txn(TxnId id) : txnId(id), committed(false) {}

    void AddOp(const Operation &op) { operations.push_back(op); }
};


struct TxnResult {
    struct ReadRes {
        KeyType key;
        ValueType val;
        TxnStamp stamp;

        ReadRes(KeyType key, ValueType val, TxnStamp stamp) : key(key), val(val), stamp(stamp) {}
    };

    TxnId txnId;
    std::vector<ReadRes> readRes;
    TxnStamp startStamp, endStamp;
};

inline std::ostream &operator<<(std::ostream &output, const TxnResult::ReadRes &readRes) {
    output << readRes.key << "'s value is " << readRes.val
           << ". Operation's timestamp is " << readRes.stamp;
}

inline std::ostream &operator<<(std::ostream &output, const TxnResult &txnResult) {
    output << txnResult.txnId << ", BEGIN, " << txnResult.startStamp << ", ," << std::endl;
    for (auto &res: txnResult.readRes) {
        output << txnResult.txnId << ", " << res.key << ", " << res.stamp << ", " << res.val << std::endl;
    }
    output << txnResult.txnId << ", END, " << txnResult.endStamp << ", ," << std::endl;
    return output;
}

struct TxnLog {
    TxnId id;
    ValueType val;
    TxnStamp stamp;
    bool committed;

    TxnLog(TxnId id, ValueType val, TxnStamp stamp, bool committed = false) :
            id(id), val(val), stamp(stamp), committed(committed) {}
};

class Parser;

class Database;

class MemoryDB;

class TxnLogBuffer;

class LockManager;

}

#endif //MVCC_FWD_H
