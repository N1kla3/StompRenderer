#pragma once
#include <deque>
#include <memory>
#include <type_traits>

namespace omp
{
    class ICommand
    {
    public:
        virtual void execute() = 0;
        virtual void undo() = 0;
        virtual ~ICommand() = default;
    protected:
        ICommand() = default;
    };
        
    template<typename T>
    concept BaseOfCommand =
    std::is_base_of_v<omp::ICommand, T>;

    class CommandStack
    {
    private:
        std::deque<std::unique_ptr<ICommand>> m_MainStack;
        std::deque<std::unique_ptr<ICommand>> m_ReverseStack;
        
        const size_t m_MaxStackSize = 20;

    public:

        template<BaseOfCommand T, typename ...Arg>
        void execute(Arg&&... arg)
        {
            // TODO: maybe thinkg of custom memory realloc
            std::unique_ptr<ICommand> new_object = std::make_unique<T>(std::forward<Arg>(arg)...);
            new_object->execute();
            m_MainStack.push_front(std::move(new_object));
            if (m_MainStack.size() > m_MaxStackSize)
            {
                m_MainStack.pop_back();
            }
        }

        void undo()
        {
            if (!m_MainStack.empty())
            {
                auto& command = m_MainStack.front();
                command->undo();
                swapBetweenStacks(m_MainStack, m_ReverseStack);
            }
        }

        void redo()
        {
            if (!m_ReverseStack.empty())
            {
                auto& command = m_ReverseStack.front();
                command->execute();
                swapBetweenStacks(m_ReverseStack, m_MainStack);
            }
        }

    private:
        void swapBetweenStacks(std::deque<std::unique_ptr<ICommand>>& fromRef, std::deque<std::unique_ptr<ICommand>>& toRef)
        {
            // TODO: add assert
            auto& command = fromRef.front();
            toRef.push_front(std::move(command));
            fromRef.pop_front();

            if (fromRef.size() > m_MaxStackSize)
            {
                fromRef.pop_back();
            }
            if (toRef.size() > m_MaxStackSize)
            {
                toRef.pop_back();
            }
        }
    };

    class CommandStackProxy
    {
    private:
        std::weak_ptr<CommandStack> m_Proxy;
    public:
        CommandStackProxy() = delete;
        CommandStackProxy(const std::shared_ptr<CommandStack>& proxy)
            : m_Proxy(proxy)
        {
            
        }

        template< typename T, typename ...Arg >
        bool execute(Arg&&... arg)
        {
            if (m_Proxy.expired())
            {
                return false;
            }
            m_Proxy.lock()->execute<T>(std::forward<Arg>(arg)...);
            return true;
        }
    };
}

