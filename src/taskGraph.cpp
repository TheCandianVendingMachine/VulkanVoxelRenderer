#include "taskGraph.hpp"
#include <assert.h>

void taskGraph::initThreadPool(unsigned int threadCount)
    {
        assert(!m_executing);
        m_threadPool.resize(threadCount);
    }

void taskGraph::initNodePool(unsigned int expectedNodeCount)
    {
        assert(!m_executing);

        m_nodePool.resize(expectedNodeCount);
        for (unsigned int i = 0; i < expectedNodeCount; i++)
            {
                m_nodePool[i] = std::make_unique<node>();
                m_nodePoolFreeIndices.push(i);
            }
    }

std::size_t taskGraph::createNode(task task)
    {
        assert(!m_executing);

        if (m_nodePoolFreeIndices.empty())
            {
                std::size_t newSize = m_nodePool.size() * 2;
                std::size_t oldSize = m_nodePool.size();
                m_nodePool.resize(newSize);
                for (std::size_t i = oldSize; i < newSize; i++)
                    {
                        m_nodePool[i] = std::make_unique<node>();
                        m_nodePoolFreeIndices.push(i);
                    }
            }

        std::size_t index = m_nodePoolFreeIndices.front();
        m_nodePoolFreeIndices.pop();
        m_nodePool[index]->m_task = task;
        m_nodePool[index]->m_doneExecution = false;
        m_nodePool[index]->m_inUse = true;
        m_nodePool[index]->m_parentsDone = 0;
        m_nodePool[index]->m_queued = false;
        return index;
    }

taskGraph::taskGraph(unsigned int expectedNodeCount)
    {
        initThreadPool(c_DEFAULT_THREAD_COUNT);
        initNodePool(expectedNodeCount);
    }

taskGraph::taskGraph(unsigned int threadCount, unsigned int expectedNodeCount)
    {
        initThreadPool(threadCount);
        initNodePool(expectedNodeCount);
    }

taskGraph::~taskGraph()
    {
        stop();
    }

taskGraph::node *taskGraph::addTask(task task, taskGraph::node *required)
    {
        std::size_t index = createNode(task);

        if (required)
            {
                m_nodePool[index]->m_parents.push_back(required);
                required->m_children.push_back(m_nodePool[index].get());
            }

        return m_nodePool[index].get();
    }

taskGraph::node *taskGraph::addTask(task task, std::initializer_list<taskGraph::node*> parents, std::initializer_list<taskGraph::node*> children)
    {
        std::size_t index = createNode(task);

        addChildren(m_nodePool[index].get(), children);
        addParents(m_nodePool[index].get(), parents);
        
        return m_nodePool[index].get();
    }

void taskGraph::addParents(taskGraph::node *base, std::initializer_list<taskGraph::node *> parents)
    {
        for (const auto &parent : parents)
            {
                parent->m_children.push_back(base);
                base->m_parents.push_back(parent);
            }
    }

void taskGraph::addChildren(taskGraph::node *base, std::initializer_list<taskGraph::node *> children)
    {
        for (const auto &child : children)
            {
                base->m_children.push_back(child);
                child->m_parents.push_back(base);
            }
    }

void taskGraph::clear()
    {
        assert(done());

        while (!m_nodePoolFreeIndices.empty())
            {
                m_nodePoolFreeIndices.pop();
            }

        for (std::size_t i = 0; i < m_nodePool.size(); i++)
            {
                m_nodePoolFreeIndices.push(i);
            }

        m_enqueuedNodes.clear();
        m_executing = false;
    }

void taskGraph::start(unsigned int threadCount)
    {
        initThreadPool(threadCount);
    }

void taskGraph::execute()
    {
        m_executing = true;
        for (auto &node : m_nodePool)
            {
                if (node->m_inUse && node->m_parents.size() == 0)
                    {
                        m_enqueuedNodes.push_back(node.get());
                    }
            }
        std::vector<node*> nextQueue;
        std::vector<std::size_t> doneNodes;

        while (!m_enqueuedNodes.empty())
            {
                std::size_t i = 0;
                for (auto &enqueuedNode : m_enqueuedNodes)
                    {
                        if (enqueuedNode->m_parentsDone >= enqueuedNode->m_parents.size())
                            {
                                std::size_t best = m_threadPool[0].m_enqueuedNodes.size();
                                executionHandler *bestHandler = &m_threadPool[0];
                                for (std::size_t j = 1; j < m_threadPool.size(); j++)
                                    {
                                        if (m_threadPool[j].m_enqueuedNodes.size() < best)
                                            {
                                                best = m_threadPool[j].m_enqueuedNodes.size();
                                                bestHandler = &m_threadPool[j];
                                            }
                                    }

                                bestHandler->m_queueMutex.lock();
                                bestHandler->m_enqueuedNodes.push(enqueuedNode);
                                bestHandler->m_queueMutex.unlock();

                                nextQueue.insert(nextQueue.end(), enqueuedNode->m_children.begin(), enqueuedNode->m_children.end());
                                doneNodes.push_back(i);
                            }
                        i++;
                    }

                if (!doneNodes.empty())
                    {
                        for (auto it = doneNodes.rbegin(); it != doneNodes.rend(); ++it)
                            {
                                m_enqueuedNodes.erase(m_enqueuedNodes.begin() + *it);
                            }
                        doneNodes.clear();
                    }

                if (!nextQueue.empty())
                    {
                        for (auto &next : nextQueue)
                            {
                                if (!next->m_queued)
                                    {
                                        m_enqueuedNodes.emplace_back(std::move(next));
                                        next->m_queued = true;
                                    }
                            }
                        nextQueue.clear();
                    }
            }

        bool isProcessing = true;
        while (isProcessing)
            {
                isProcessing = false;
                for (auto &executor : m_threadPool)
                    {
                        if (executor.m_isProcessing || !executor.m_enqueuedNodes.empty())
                            {
                                isProcessing = true;
                            }
                    }
            }

        m_executing = false;
    }

bool taskGraph::done() const
    {
        return !m_executing;
    }

void taskGraph::stop()
    {
        for (auto &thread : m_threadPool)
            {
                thread.m_running = false;
            }
        for (auto &thread : m_threadPool)
            {
                thread.m_thread.join();
            }
        m_threadPool.clear();
        m_executing = false;
    }

taskGraph &taskGraph::operator=(taskGraph &rhs)
    {
        if (&rhs == this)
            {
                return *this;
            }

        m_nodePool.clear();
        for (auto &node : rhs.m_nodePool)
            {
                m_nodePool.emplace_back(node.release());
            }
        m_nodePoolFreeIndices = rhs.m_nodePoolFreeIndices;
        m_enqueuedNodes = rhs.m_enqueuedNodes;
        m_threadPool = rhs.m_threadPool;
        m_executing = rhs.m_executing;

        return *this;
    }

taskGraph::executionHandler::executionHandler()
    {
        m_running = true;
        m_thread = std::thread(&taskGraph::executionHandler::executionLoop, this);
    }

taskGraph::executionHandler::executionHandler(const executionHandler &rhs)
    {
        *this = rhs;
    }

void taskGraph::executionHandler::executionLoop()
    {
        while (m_running)
            {
                if (m_enqueuedNodes.empty())
                    {
                        m_isProcessing = false;
                        std::this_thread::yield();
                    }
                else
                    {
                        m_isProcessing = true;

                        m_queueMutex.lock();
                        taskGraph::node *front = m_enqueuedNodes.front();
                        m_queueMutex.unlock();

                        front->execute();

                        m_queueMutex.lock();
                        m_enqueuedNodes.pop();
                        m_queueMutex.unlock();
                    }
            }
    }

taskGraph::executionHandler &taskGraph::executionHandler::operator=(const executionHandler &rhs)
    {
        if (this == &rhs)
            {
                return *this;
            }

        m_enqueuedNodes = rhs.m_enqueuedNodes;
        m_running = rhs.m_running;
        m_thread = std::thread(&taskGraph::executionHandler::executionLoop, this);

        return *this;
    }
