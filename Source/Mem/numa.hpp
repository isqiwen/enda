#pragma once

#include <algorithm> // for max
#include <cerrno>    // for ENOSYS, EXDEV, errno
#include <climits>   // for INT_MAX
#include <cstddef>   // for size_t
#include <iterator>  // for distance
#include <map>       // for map, operator==
#include <memory>    // for shared_ptr, operator==, unique_ptr
#include <set>       // for set
#include <stdexcept> // for runtime_error
#include <utility>   // for move
#include <vector>    // for vector

#include "macros.hpp" // for ENDA_ASSERT, ENDA_STATIC_CALL, ENDA_STATIC_CONST

#ifdef __has_include
    #if defined(ENDA_USE_HWLOC) && not __has_include(<hwloc.h>)
        #error "ENDA_USE_HWLOC is defined but <hwloc.h> is not available"
    #endif
#endif

#ifdef ENDA_USE_HWLOC
    #include <hwloc.h> // for hwloc_obj, hwloc_obj_t, hwl...
#endif

#ifdef ENDA_USE_HWLOC
static_assert(HWLOC_VERSION_MAJOR == 2, "hwloc too old");
#endif

/**
 * @brief An opaque description of a set of processing units.
 *
 * \rst
 *
 * .. note::
 *
 *    If your system is missing `hwloc` then this type will be incomplete.
 *
 * \endrst
 */
struct hwloc_bitmap_s;
/**
 * @brief An opaque description of a computers topology.
 *
 * \rst
 *
 * .. note::
 *
 *    If your system is missing `hwloc` then this type will be incomplete.
 *
 * \endrst
 */
struct hwloc_topology;

namespace enda::mem
{
    struct hwloc_error : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    /**
     * @brief Enum to control distribution strategy of workers among numa nodes.
     */
    enum class numa_strategy
    {
        /**
         * @brief Put workers as far away from each other as possible (maximize cache.)
         */
        fan,
        /**
         * @brief Fill up each numa node sequentially (ignoring SMT).
         */
        seq,
    };

    /**
     * @brief A shared description of a computers topology.
     *
     * Objects of this type have shared-pointer semantics.
     */
    class numa_topology
    {
    private:
        struct bitmap_deleter
        {
            ENDA_STATIC_CALL void operator()(hwloc_bitmap_s* ptr) ENDA_STATIC_CONST noexcept
            {
#ifdef ENDA_USE_HWLOC
                hwloc_bitmap_free(ptr);
#else
                ENDA_ASSERT(!ptr);
#endif
            }
        };

        using unique_cpup = std::unique_ptr<hwloc_bitmap_s, bitmap_deleter>;

        using shared_topo = std::shared_ptr<hwloc_topology>;

    public:
        /**
         * @brief Construct a topology.
         *
         * If `hwloc` is not installed this topology is empty.
         */
        numa_topology();

        /**
         * @brief Test if this topology is empty.
         */
        explicit operator bool() const noexcept { return m_topology != nullptr; }

        /**
         * A handle to a single processing unit in a NUMA computer.
         */
        class numa_handle
        {
        public:
            /**
             * @brief Bind the calling thread to the set of processing units in this `cpuset`.
             *
             * If `hwloc` is not installed both handles are null and this is a noop.
             */
            void bind() const;

            /**
             * @brief A shared handle to topology this handle belongs to.
             */
            shared_topo topo = nullptr;
            /**
             * @brief A unique handle to processing units that this handle represents.
             */
            unique_cpup cpup = nullptr;
            /**
             * @brief  The index of the numa node this handle belongs to, on [0, n).
             */
            std::size_t numa = 0;
        };

        /**
         * @brief Split a topology into `n` uniformly distributed handles to single
         * processing units.
         *
         * Here the definition of "uniformly" depends on `strategy`. If `strategy == numa_strategy::seq` we try
         * to use the minimum number of numa nodes then divided each node such that each PU has as much cache as
         * possible. If `strategy == numa_strategy::fan` we try and maximize the amount of cache each PI gets.
         *
         * If this topology is empty then this function returns a vector of `n` empty handles.
         */
        auto split(std::size_t n, numa_strategy strategy = numa_strategy::fan) const -> std::vector<numa_handle>;

        /**
         * @brief A single-threads hierarchical view of a set of objects.
         *
         * This is a `numa_handle` augmented with  list of neighbors-lists each
         * neighbors-list has equidistant neighbors. The first neighbors-list always
         * exists and contains only one element, the one "owned" by the thread. Each
         * subsequent neighbors-list has elements that are topologically more distant
         * from the element in the first neighbour-list.
         */
        template<typename T>
        struct numa_node : numa_handle
        {
            /**
             * @brief A list of neighbors-lists.
             */
            std::vector<std::vector<std::shared_ptr<T>>> neighbors;
        };

        /**
         * @brief Distribute a vector of objects over this topology.
         *
         * This function returns a vector of `numa_node`s. Each `numa_node` contains a
         * hierarchical view of the elements in `data`.
         */
        template<typename T>
        auto distribute(std::vector<std::shared_ptr<T>> const& data, numa_strategy strategy = numa_strategy::fan) -> std::vector<numa_node<T>>;

    private:
        shared_topo m_topology = nullptr;
    };

    // ---------------------------- Topology implementation ---------------------------- //

#ifdef ENDA_USE_HWLOC

    inline numa_topology::numa_topology()
    {
        struct topology_deleter
        {
            ENDA_STATIC_CALL void operator()(hwloc_topology* ptr) ENDA_STATIC_CONST noexcept
            {
                if (ptr != nullptr)
                {
                    hwloc_topology_destroy(ptr);
                }
            }
        };

        hwloc_topology* tmp = nullptr;

        if (hwloc_topology_init(&tmp) != 0)
        {
            throw hwloc_error("failed to initialize a topology");
        }

        m_topology = {tmp, topology_deleter {}};

        if (hwloc_topology_load(m_topology.get()) != 0)
        {
            throw hwloc_error("failed to load a topology");
        }
    }

    inline void numa_topology::numa_handle::bind() const
    {
        ENDA_ASSERT(topo);
        ENDA_ASSERT(cpup);

        switch (hwloc_set_cpubind(topo.get(), cpup.get(), HWLOC_CPUBIND_THREAD))
        {
            case 0:
                return;
            case -1:
                switch (errno)
                {
                    case ENOSYS:
                        throw hwloc_error("hwloc's cpu binding is not supported on this system");
                    case EXDEV:
                        throw hwloc_error("hwloc cannot enforce the requested binding");
                    default:
                        throw hwloc_error("hwloc cpu bind reported an unknown error");
                }
            default:
                throw hwloc_error("hwloc cpu bind returned un unexpected value");
        }
    }

    inline auto count_cores(hwloc_obj_t obj) -> unsigned int
    {
        ENDA_ASSERT(obj);

        if (obj->type == HWLOC_OBJ_CORE)
        {
            return 1;
        }

        unsigned int num_cores = 0;

        for (unsigned int i = 0; i < obj->arity; i++)
        {
            num_cores += count_cores(obj->children[i]);
        }

        return num_cores;
    }

    inline auto get_numa_index(hwloc_topology* topo, hwloc_bitmap_s* bitmap) -> hwloc_uint64_t
    {
        ENDA_ASSERT(topo);
        ENDA_ASSERT(bitmap);

        hwloc_obj* obj = hwloc_get_obj_covering_cpuset(topo, bitmap);

        if (obj == nullptr)
        {
            throw hwloc_error("failed to find an object covering a bitmap");
        }

        while (obj != nullptr && obj->memory_arity == 0)
        {
            obj = obj->parent;
        }

        if (obj == nullptr)
        {
            throw hwloc_error("failed to find a parent with memory");
        };

        return obj->gp_index;
    }

#else

    inline numa_topology::numa_topology() : m_topology {nullptr, [](hwloc_topology* ptr) { ENDA_ASSERT(!ptr); }} {}

    inline void numa_topology::numa_handle::bind() const
    {
        ENDA_ASSERT(!topo);
        ENDA_ASSERT(!cpup);
    }

    inline auto numa_topology::split(std::size_t n, numa_strategy /* strategy */) const -> std::vector<numa_handle> { return std::vector<numa_handle>(n); }

    template<typename T>
    inline auto numa_topology::distribute(const std::vector<std::shared_ptr<T>>& data, numa_strategy strategy) -> std::vector<numa_node<T>>
    {
        std::vector<numa_handle> handles = split(data.size(), strategy);

        std::vector<numa_node<T>> views;

        for (std::size_t i = 0; i < data.size(); i++)
        {
            numa_node<T> node {
                std::move(handles[i]), {{data[i]}}, // The first neighbors-list contains
                                                    // only the object itself.
            };

            if (data.size() > 1)
            {
                node.neighbors.push_back({});
            }

            for (auto const& neigh : data)
            {
                if (neigh != data[i])
                {
                    node.neighbors[1].push_back(neigh);
                }
            }

            views.push_back(std::move(node));
        }

        return views;
    }

#endif

} // namespace enda::mem
