// taskGraph.hpp
// Allows for creation of a execution graph. Will be executed when desired and will run through behaviours logically
#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <memory>
#include <initializer_list>
#include <mutex>
#include "task.hpp"

class taskGraph
    {
        private:
            struct node
                {
                    std::vector<node*> m_parents;
                    std::vector<node*> m_children;
                    task m_task;
                    bool m_doneExecution = false;
                    bool m_inUse = false;
                    bool m_queued = false;
                    std::atomic<int> m_parentsDone = 0;

                    void execute()
                        {
                            m_task.execute();
                            for (auto &child : m_children)
                                {
                                    child->m_parentsDone++;
                                }
                            m_doneExecution = true;
                        }

                    node() = default;
                    node(node &rhs)
                        {
                            *this = rhs;
                        }

                    node(node &&rhs) noexcept
                        {
                            *this = std::move(rhs);
                        }

                    node &operator=(node &rhs)
                        {
                            if (&rhs == this)
                                {
                                    return *this;
                                }
                            m_parents = rhs.m_parents;
                            m_children = rhs.m_children;
                            m_task = rhs.m_task;
                            m_doneExecution = rhs.m_doneExecution;
                            m_inUse = rhs.m_inUse;
                            m_parentsDone.store(rhs.m_parentsDone);

                            return *this;
                        }

                    node &operator=(node &&rhs) noexcept
                        {
                            if (&rhs == this)
                                {
                                    return *this;
                                }

                            m_parents = std::move(rhs.m_parents);
                            m_children = std::move(rhs.m_children);
                            m_task = std::move(rhs.m_task);
                            m_doneExecution = std::move(rhs.m_doneExecution);
                            m_inUse = std::move(rhs.m_inUse);
                            m_parentsDone.store(rhs.m_parentsDone);

                            return *this;
                        }
                };

            struct executionHandler
                {
                    std::queue<node*> m_enqueuedNodes;
                    std::mutex m_queueMutex;
                    std::thread m_thread;
                    bool m_running = false;
                    std::atomic<bool> m_isProcessing = false;

                    executionHandler();
                    executionHandler(const executionHandler &rhs);
                    void executionLoop();

                    executionHandler &operator=(const executionHandler &rhs);
                };

            std::vector<std::unique_ptr<node>> m_nodePool;
            std::queue<std::size_t> m_nodePoolFreeIndices;

            std::vector<node*> m_enqueuedNodes;

            std::vector<executionHandler> m_threadPool;
            static constexpr unsigned int c_DEFAULT_THREAD_COUNT = 4;

            bool m_executing = false;
            friend class taskGraphTests;
            friend class taskGraphTests_add_Test;
            friend class taskGraphTests_clear_Test;
            friend class taskGraphTests_stop_Test;
            friend class taskGraphTests_executeSimple_Test;
            friend class taskGraphTests_executeComplex_Test;
            friend class taskGraphTests_executeComplexNoDelay_Test;

            void initThreadPool(unsigned int threadCount);
            void initNodePool(unsigned int expectedNodeCount);
            std::size_t createNode(task task);

        public:
            taskGraph(unsigned int expectedNodeCount = 10);
            taskGraph(unsigned int threadCount, unsigned int expectedNodeCount = 10);
            ~taskGraph();

            taskGraph::node *addTask(task task, taskGraph::node *required = nullptr);
            taskGraph::node *addTask(task task, std::initializer_list<taskGraph::node*> parents, std::initializer_list<taskGraph::node*> children);

            void addParents(taskGraph::node *base, std::initializer_list<taskGraph::node*> parents);
            void addChildren(taskGraph::node *base, std::initializer_list<taskGraph::node*> children);

            void clear();

            void start(unsigned int threadCount);
            void execute();
            bool done() const;
            void stop();

            taskGraph &operator=(taskGraph &rhs);

    };
