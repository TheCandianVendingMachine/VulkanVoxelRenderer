// task.hpp
// Represents a single task within the graph. Binds either a method with an object or a function and allows the user to execute it without needing to specify the template type beforehand
#pragma once
#include <memory>
#include <cstddef>
#include <functional>

class task
    {
        private:
            template<typename TFunc>
            struct taskFunc;

            template<typename TObj, typename TFunc>
            struct taskFuncMethod;

            struct taskFuncBase
                {
                    std::size_t m_functionOffset = 0;
                    template<typename TRet, typename ...TArgs>
                    auto execute(TArgs ...args)
                        {
                            std::uint8_t *functionOffset = reinterpret_cast<std::uint8_t*>(this) + m_functionOffset;
                            return (*reinterpret_cast<std::function<TRet(TArgs...)>*>(functionOffset))(args...);
                        }
                };
            
            template<typename TFuncResult, typename ...TFuncArgs>
            struct taskFunc<TFuncResult(TFuncArgs...)> : taskFuncBase
                {
                    TFuncResult(*m_functionPtr)(TFuncArgs...);
                    std::function<TFuncResult(TFuncArgs...)> m_execute;

                    taskFunc(TFuncResult(*func)(TFuncArgs...))
                        {
                            m_execute = [this] (TFuncArgs ...args) {
                                return m_functionPtr(args...);
                            };

                            m_functionPtr = func;
                            m_functionOffset = offsetof(taskFunc, m_execute);

                            TFuncResult();
                        }
                };

            template<typename TObj, typename TFuncResult, typename TBaseObj, typename ...TFuncArgs>
            struct taskFuncMethod<TObj, TFuncResult(TBaseObj::*)(TFuncArgs...)> : taskFuncBase
                {
                    TFuncResult(TBaseObj::*m_functionPtr)(TFuncArgs...);
                    TObj *m_attachedObject;
                    std::function<TFuncResult(TFuncArgs...)> m_execute;

                    taskFuncMethod(TObj *attached, TFuncResult(TBaseObj::*func)(TFuncArgs...))
                        {
                            m_execute = [this] (TFuncArgs ...args) {
                                return (m_attachedObject->*m_functionPtr)(args...);
                            };

                            m_attachedObject = attached;
                            m_functionPtr = func;
                            m_functionOffset = offsetof(taskFuncMethod, m_execute);
                        }
                };

            std::unique_ptr<taskFuncBase> m_taskFunc;

        public:
            task() = default;
            task(task &rhs)
                {
                    *this = rhs;
                }

            task(task &&rhs) noexcept
                {
                    *this = std::move(rhs);
                }

            template<typename TFunc>
            task(TFunc &func)
                {
                    assign(func);
                }

            template<typename TObj, typename TFunc>
            task(TObj *attached, TFunc func)
                {
                    assign(attached, func);
                }

            template<typename TFunc>
            void assign(TFunc &func)
                {
                    m_taskFunc = std::make_unique<taskFunc<TFunc>>(func);
                }

            template<typename TObj, typename TFunc>
            void assign(TObj *attached, TFunc func)
                {
                    m_taskFunc = std::make_unique<taskFuncMethod<TObj, TFunc>>(attached, func);
                }

            template<typename TRet = void, typename ...Args>
            auto execute(Args ...args)
                {
                    return m_taskFunc->execute<TRet>(args...);
                }

            task &operator=(task &rhs)
                {
                    if (&rhs == this)
                        {
                            return *this;
                        }
                    m_taskFunc.reset(rhs.m_taskFunc.release());
                    return *this;
                }

            task &operator=(task &&rhs) noexcept
                {
                    if (&rhs == this)
                        {
                            return *this;
                        }
                    m_taskFunc = std::move(rhs.m_taskFunc);
                    return *this;
                }

    };
