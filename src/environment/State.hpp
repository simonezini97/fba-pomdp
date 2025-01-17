#ifndef STATE_HPP
#define STATE_HPP

#include <cassert>

#include <string>

/**
 * @brief A description of a single state of the environment
 **/
class State
{
public:
    virtual ~State() = default;

    virtual void index(int i) = 0;

    /**
     * @brief gets the index of the state
     *
     * May be overridden by derived classes if calculated on the spot
     **/
    virtual int index() const = 0;

    /**
     * @brief gets state information in string form
     *
     * May be overwritten by derived classes.
     * Returns index string by default.
     **/
    virtual std::string toString() const = 0;
};

#include "utils/IndexedElements.hpp"
using IndexState = indexing::IndexedElement<State>;

#endif // STATE_HPP
