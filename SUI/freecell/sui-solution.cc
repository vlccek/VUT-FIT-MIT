#include <queue>
#include <set>
#include <unordered_set>
#include <algorithm>

#include <vector>
#include <queue>
#include <set>
#include <map>
#include <unordered_set>
#include <stack>
#include <sstream>
#include <string>

#include <utility>

#include "search-strategies.h"
#include "memusage.h"

typedef std::shared_ptr<SearchState> SearchSharedPtr;

constexpr std::size_t RESERVE =50000000; // 50 MB memory reserve

inline bool memUsageSucceeded(std::size_t limit) {
    return getCurrentRSS() > limit - RESERVE;
}


typedef int my_hash_type;
inline my_hash_type my_hash(const SearchState &state){
    // creates hash from state as a string
    std::stringstream state_string;
    state_string << state;
    return std::hash<std::string>{}(state_string.str());
}


std::vector<SearchAction> BreadthFirstSearch::solve(const SearchState &init_state) {
    std::queue<std::vector<SearchAction>> action_queue;
    std::set<my_hash_type> visited_hashes;

    my_hash_type init_hash = my_hash(init_state);
    visited_hashes.insert(init_hash);
    action_queue.emplace();

    while (!action_queue.empty()) {
        auto current_path = std::move(action_queue.front());
        action_queue.pop();

        SearchState current_state = init_state;
        for (const SearchAction& action : current_path) {
            current_state = action.execute(current_state);
        }

        if (getCurrentRSS() > mem_limit_ - RESERVE) {
            
            break;
        }

        if (current_state.isFinal()) {
            
            return current_path;
        }

        for (const SearchAction& action : current_state.actions()) {
            SearchState new_state = action.execute(current_state);

            if (new_state.isFinal()) {
                
                std::vector<SearchAction> new_path = current_path;
                new_path.push_back(action);
                return new_path;
            }


            my_hash_type new_state_hash = my_hash(new_state);
            if (visited_hashes.insert(new_state_hash).second) {
                std::vector<SearchAction> new_path = current_path;
                new_path.push_back(action);
                action_queue.emplace(std::move(new_path));
            }
        }
    }


    return {};
}



std::vector<SearchAction> DepthFirstSearch::solve(const SearchState& init_state)
{
    std::stack<std::vector<SearchAction>> action_queue;

    action_queue.emplace();

    while (!action_queue.empty()) {
        auto current_path = std::move(action_queue.top());
        action_queue.pop();

        if (current_path.size() >= depth_limit_) {
            continue;
        }

        // Rekonstrukce aktuálního stavu na základě cesty akcí
        SearchState current_state = init_state;
        for (const SearchAction& action : current_path) {
            current_state = action.execute(current_state);
        }

        // Kontrola paměťového limitu
        if (getCurrentRSS() > mem_limit_ - RESERVE) {
            
            break;
        }

        // Kontrola, zda je stav finální
        if (current_state.isFinal()) {
            
            return current_path;
        }

        // Generování všech možných následujících stavů
        for (const SearchAction& action : current_state.actions()) {
            SearchState new_state = action.execute(current_state);

            if (new_state.isFinal()) {
                
                std::vector<SearchAction> new_path = current_path;
                new_path.push_back(action);
                return new_path;
            }

            std::vector<SearchAction> new_path = current_path;
            new_path.push_back(action);
            action_queue.emplace(std::move(new_path));
        }
    }


    return {}; // Vrací prázdný vektor, pokud řešení neexistuje
}


double StudentHeuristic::distanceLowerBound(const GameState& state) const
{
    return 0;
}


struct PriorityQueueNode {
    std::vector<SearchAction> path;
    double priority;

    // Operátor pro porovnání ve frontě (nižší priorita má vyšší prioritu)
    bool operator<(const PriorityQueueNode& other) const {
        return priority > other.priority;
    }
};


std::vector<SearchAction> AStarSearch::solve(const SearchState& init_state) {

    std::priority_queue<PriorityQueueNode> action_queue; // Prioritní fronta
    std::unordered_set<my_hash_type> visited_hashes;     // Množina navštívených stavů

    my_hash_type init_hash = my_hash(init_state);
    visited_hashes.insert(init_hash);

    // Inicializace prioritní fronty
    action_queue.push({{}, compute_heuristic(init_state, *heuristic_)});

    while (!action_queue.empty()) {
        auto current_node = action_queue.top();
        action_queue.pop();

        const auto& current_path = current_node.path;

        // Rekonstrukce aktuálního stavu na základě cesty akcí
        SearchState current_state = init_state;
        for (const SearchAction& action : current_path) {
            current_state = action.execute(current_state);
        }

        // Kontrola paměťového limitu
        if (getCurrentRSS() > mem_limit_ - RESERVE) {
            
            break;
        }

        // Kontrola, zda je stav finální
        if (current_state.isFinal()) {
            
            return current_path;
        }

        // Generování všech možných následujících stavů
        for (const SearchAction& action : current_state.actions()) {
            SearchState new_state = action.execute(current_state);

            if (new_state.isFinal()) {
                
                std::vector<SearchAction> new_path = current_path;
                new_path.push_back(action);
                return new_path;
            }

            my_hash_type new_state_hash = my_hash(new_state);
            if (visited_hashes.insert(new_state_hash).second) {
                std::vector<SearchAction> new_path = current_path;
                new_path.push_back(action);

                // Výpočet skóre (g + h)
                double g_cost = new_path.size(); // Délka cesty jako cena
                double h_cost = compute_heuristic(new_state, *heuristic_);
                double f_cost = g_cost + h_cost;

                action_queue.push({std::move(new_path), f_cost});
            }
        }
    }


    return {}; // Vrací prázdný vektor, pokud řešení neexistuje
}