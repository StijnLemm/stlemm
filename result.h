#pragma once

#include "base.h"
#include "memory.h"

template <typename V, typename E>
class Result
{
public:
    implicit Result(V value) : _has_value(true)
    {
        new (&_value) V(Memory::move(value));
    }

    implicit Result(E value) : _has_value(false)
    {
        new (&_error) E(value);
    }

    ~Result()
    {
        if (_has_value)
        {
            _value.~V();
        }
        else
        {
            _error.~E();
        }
    }

    constexpr operator bool() const
    {
        return _has_value;
    }

    V unwrap()
    {
        expect(_has_value, "Tried to unwrap error on error-result!");
        return Memory::move(_value);
    }

    E unwrap_err()
    {
        expect(!_has_value, "Tried to unwrap error on value-result!");
        return Memory::move(_error);
    }

    template <typename ValueFnT, typename ErrorFnT>
    void visit(ValueFnT&& onValue, ErrorFnT&& onError)
    {
        if (_has_value)
        {
            onValue(Memory::move(_value));
        }
        else
        {
            onError(Memory::move(_error));
        }
    }

private:
    union
    {
        V _value;
        E _error;
    };
    const bool _has_value;
};

enum class ResultError
{
    FAIL,
};
template <typename T>
using DefaultResult = Result<T, ResultError>;
