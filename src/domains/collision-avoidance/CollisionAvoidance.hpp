#ifndef COLLISIONAVOIDANCE_HPP
#define COLLISIONAVOIDANCE_HPP

#include "domains/POMDP.hpp"

#include <memory>
#include <random>
#include <vector>

#include "environment/Action.hpp"
#include "environment/Observation.hpp"
#include "environment/Reward.hpp"
#include "environment/State.hpp"
#include "environment/Terminal.hpp"
#include "utils/DiscreteSpace.hpp"
#include "utils/distributions.hpp"
#include "utils/random.hpp"

namespace domains {

/**
 * @brief The collision avoidance domain
 *
 * Consist of the <x,y> position of the agent and y position of the obstacles
 **/
struct CollisionAvoidanceState : public State
{

    CollisionAvoidanceState(
        int x_agent_in,
        int y_agent_in,
        // cppcheck-suppress passedByValue
        std::vector<int> obstacles_pos_in,
        int index) :
            x_agent(x_agent_in),
            y_agent(y_agent_in),
            _index(index),
            obstacles_pos(std::move(obstacles_pos_in))
    {
    }

    void index(int /*i*/) final
    {
        throw "CollisionAvoidanceState::index(i) should not be called...?";
    }

    int index() const final { return _index; }

    std::string toString() const final
    {
        std::string obsts = "{";

        for (size_t i = 0; i < obstacles_pos.size() - 1; ++i)
        { obsts += std::to_string(obstacles_pos[i]) + ","; }

        return "(index:" + std::to_string(_index) + " (" + std::to_string(x_agent) + ","
               + std::to_string(y_agent) + "), " + obsts + std::to_string(obstacles_pos.back())
               + "})";
    }

    int x_agent, y_agent, _index;

    std::vector<int> obstacles_pos;
};

/** \brief The collision avoidance domain
  >>>>>>> domain_ba_separation
 *
 * The agent starts moving from a random position in the right-most column
 * of a x by y 30 grid map. An obstacle randomly moves in the left-most
 * column of this map: moves up with probability 0.25, down with 0.25, and
 * stay put with 0.50. The probabilities become 0, 0.25, and 0.75
 * respectively when the obstacle is at the topmost row, and become 0.25,
 * 0, and 0.75 respectively when it is at the bottom-most row. Each time,
 * the agent can choose to move upper-left, lower-left or left, with a cost
 * of -1,-1 and 0 respectively. If the agent collides with the obstacle, it
 * receives a penalty of -1000. The task finishes when the agent reaches
 * the left-most column. The agent knows its own position exactly, but
 * observes the obstacle’s position with a Gaussian noise N(0; 1).
 *
 **/
class CollisionAvoidance : public POMDP
{

public:
    enum VERSION { INIT_RANDOM_POSITION, INITIALIZE_CENTRE };
    enum move { MOVE_DOWN, STAY, MOVE_UP };

    constexpr static int const NUM_ACTIONS        = 3;
    constexpr static double const MOVE_PENALTY    = 1;
    constexpr static double const COLLIDE_PENALTY = 1000;
    constexpr static double const BLOCK_MOVE_PROB = .5;

    CollisionAvoidance(
        int grid_width,
        int grid_height,
        int num_obstacles = 1,
        VERSION version   = INIT_RANDOM_POSITION);
    ~CollisionAvoidance() final;

    /***** getters *****/
    /**
     * @brief returns the type of the domain
     *
     * @return VERSION::INIT_RANDOM_POSITION or VERSION::INITIALIZE_CENTRE
     */
    VERSION type() const;
    int xAgent(State const* s) const;
    int yAgent(State const* s) const;
    std::vector<int> const& yObstacles(State const* s) const;

    State const* getState(int x, int y, std::vector<int> const& obstacles) const;

    Action const* getAction(move m) const;
    Observation const* getObservation(int y) const;

    /***** domain interface ****/
    Action const* generateRandomAction(State const* s) const final;
    void addLegalActions(State const* s, std::vector<Action const*>* actions) const final;
    double computeObservationProbability(Observation const* o, Action const* a, State const* new_s)
        const final;

    Action const* copyAction(Action const* a) const final;
    void releaseAction(Action const* a) const final;

    /***** Environment interface *****/
    State const* sampleStartState() const final;
    Terminal step(State const** s, Action const* a, Observation const** o, Reward* r) const final;

    void releaseObservation(Observation const* o) const final;
    void releaseState(State const* s) const final;
    Observation const* copyObservation(Observation const* o) const final;
    State const* copyState(State const* s) const final;

private:
    int const _grid_width;
    int const _grid_height;
    int const _num_obstacles;
    VERSION const _version;

    std::vector<int> const _obstacles_space = std::vector<int>(_num_obstacles, _grid_height);

    utils::DiscreteSpace<IndexAction> _actions{NUM_ACTIONS};
    std::vector<Observation*> _observations{
        static_cast<size_t>(static_cast<int>(std::pow(_grid_height, _num_obstacles)))};

    std::vector<std::vector<std::vector<State const*>>> _states{
        static_cast<size_t>(_grid_width),
        std::vector<std::vector<State const*>>(
            _grid_height,
            std::vector<State const*>(std::pow(_grid_height, _num_obstacles)))};

    // probability of observation distance
    // element i contains the probability of observing
    // the obstacle i cells away from its actual position
    std::vector<double> _observation_error_probability{};

    // uniform distriution over the actions
    mutable std::uniform_int_distribution<int> _action_distr{
        rnd::integerDistribution(0, NUM_ACTIONS)};

    // describes the noise of the observation
    mutable std::normal_distribution<float> _observation_distr =
        std::normal_distribution<float>(0, 1);

    mutable std::uniform_int_distribution<int> _y_sampler{
        rnd::integerDistribution(0, _grid_height)};

    utils::categoricalDistr _state_prior{static_cast<size_t>(
        _grid_width * _grid_height * static_cast<int>(std::pow(_grid_height, _num_obstacles)))};

    /**
     * @brief returns the state associated with index
     *
     * @param index the index of the state
     *
     * @return the CollisionAvoidanceState of index
     */
    State const* getState(int index) const;

    /**
     * @brief computes reward associated with <*,a,new_s>
     *
     * @param a
     * @param new_s
     *
     * @return the reward of ending up in new_s after taking action a
     */
    Reward reward(Action const* a, State const* new_s) const;

    /**
     * @brief given some y value, it will make sure to place it within the grid
     *
     * If y is higher than the grid height, then it will be set to
     * the top cell. Similarly any negative value will be set to 0
     **/
    int keepInGrid(int y) const;

    /**
     * @brief returns a new position of the obstacle
     *
     * obstacle moves up & down with 25% and stays put with 50%
     */
    int moveObstacle(int current_position) const;

    void assertLegal(State const* s) const;
    void assertLegal(Action const* a) const;
    void assertLegal(Observation const* o) const;
};

} // namespace domains

#endif // COLLISIONAVOIDANCE_HPP
