#pragma once
#include <stack>
#include <memory>
#include <type_traits>

namespace omp
{
    class ICommand
    {
    public:
        virtual void execute() = 0;
        virtual void undo() = 0;
        virtual ~ICommand();
    };
        
    template<typename T>
    concept BaseOfCommand =
    std::is_base_of_v<omp::ICommand, T>;

    class CommandStack
    {
    private:
        std::stack<std::unique_ptr<ICommand>> m_MainStack;
        std::stack<std::unique_ptr<ICommand>> m_ReverseStack;

    public:

        template<BaseOfCommand T, typename ...Arg>
        void execute(Arg&&... arg)
        {
            // TODO: maybe thinkg of custom memory realloc
            std::unique_ptr<ICommand> new_object = std::make_unique<T>(std::forward<Arg>(arg)...);
            new_object->execute();
            m_MainStack.push(std::move(new_object));
        }

        void undo()
        {
            auto& command = m_MainStack.top();
            if (command)
            {
                command->undo();
                m_ReverseStack.push(std::move(command));
                m_MainStack.pop();
            }
        }

        void redo()
        {
            auto& command = m_ReverseStack.top();
            if (command)
            {
                command->execute();
                m_MainStack.push(std::move(command));
                m_ReverseStack.pop();
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

