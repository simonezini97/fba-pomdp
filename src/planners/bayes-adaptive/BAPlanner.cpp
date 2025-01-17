#include "BAPlanner.hpp"

#include "configurations/Conf.hpp"

#include "planners/bayes-adaptive/BATSPlanner.hpp"
#include "planners/bayes-adaptive/RBAPOUCT.hpp"
#include "planners/random/RandomPlanner.hpp"

namespace factory {

std::unique_ptr<Planner> makeBAPlanner(configurations::Conf const& c)
{
    if (c.planner == "random")
        return std::unique_ptr<Planner>(new planners::RandomPlanner());
    if (c.planner == "ts")
        return std::unique_ptr<Planner>(new planners::BATSPlanner(c));
    if (c.planner == "po-uct")
        return std::unique_ptr<Planner>(new planners::RBAPOUCT(c));

    throw "incorrect planner provided";
}

} // namespace factory
