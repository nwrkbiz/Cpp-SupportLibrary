/**
 * @file Observer.h
 * @brief Observer/Obersvable Pattern implementation.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */
#ifndef SUPPORTLIB_OBSERVER_OBSERVABLE_H
#define SUPPORTLIB_OBSERVER_OBSERVABLE_H
#include "Object.h"
#include <vector>
#include <algorithm>
namespace giri {
    /**
     *  @brief Observer class. Inherited classes can observe
     *  all classes which inherit from Observable.
     * 
     *  Example Usage:
     *  --------------
     * 
     *  @code{.cpp}
     *  #include <Observer.h>
     *  #include <string>
     *  #include <iostream>
     *  class MyObservable : public giri::Observable<MyObservable>
     *  {
     *  public:
     *      std::string GetObservable() const {return "Observable ;)"; };
     *      using SPtr = std::shared_ptr<MyObservable>;
     *      using UPtr = std::unique_ptr<MyObservable>;
     *      using WPtr = std::weak_ptr<MyObservable>;
     *  };
     *  // ... example to be cuntinued at Observable class documentation
     *  @endcode
     */
    template <typename U>
    class Observer : public Object< Observer<U> >
    {
    public:
        /**
         * Called by Observable on notify.
         * @param observable Object that triggered the update.
         */
        virtual void update(std::shared_ptr<U> observable) = 0;
    protected:
        Observer() = default;
    };

    /**
     *  @brief Observable class. Inherited classes can notify
     *  all classes which inherit from Observer.
     * 
     *  Example Usage:
     *  --------------
     *  @code{.cpp}
     *  // .... example starts at Observer class documentation
     *  class MyObserver : public giri::Observer<MyObservable>
     *  {
     *  public:
     *      MyObserver(const std::string & name) : m_Name(name){}
     *      void update(MyObservable::SPtr obs)
     *      {
     *          std::cout << m_Name << " Update: "<< obs->GetObservable() << std::endl;
     *      }
     *      using SPtr = std::shared_ptr<MyObserver>;
     *      using UPtr = std::unique_ptr<MyObserver>;
     *      using WPtr = std::weak_ptr<MyObserver>;
     *  private:
     *      std::string m_Name;
     *  };
     *  
     *  int main()
     *  {
     *      MyObservable::SPtr osrvbl = std::make_shared<MyObservable>();
     *      MyObserver::SPtr obs1 = std::make_shared<MyObserver>("Observer 1");
     *      MyObserver::SPtr obs2 = std::make_shared<MyObserver>("Observer 2");
     *  
     *      // subscribe
     *      osrvbl->subscribe(obs1);
     *      osrvbl->subscribe(obs2);
     *  
     *      osrvbl->notify();
     *  
     *      // Unsubscribe obs1
     *      osrvbl->unsubscribe(obs1);
     *  
     *      osrvbl->notify();
     *      return EXIT_SUCCESS;
     *  }
     *  @endcode
     */
    template <typename T>
    class Observable : public Object< Observable<T> >, public std::enable_shared_from_this<T>
    {
    public:
        /**
         * Subscribes a Observer.
         * @param obs Observer to subscribe.
         */
        void subscribe(const std::weak_ptr< Observer<T> >& obs){
            if(!obs.expired())
                m_Observers.push_back(obs);
        }
        /**
         * Unsubscribes a Observer.
         * @param obs Observer to unsubscribe.
         */
        void unsubscribe(const std::weak_ptr< Observer<T> >& obs){
            for(size_t i = 0; i < m_Observers.size(); i++)
                if(m_Observers[i].lock() == obs.lock()) {
                    m_Observers.erase(m_Observers.begin() + i);
                    break;
                }
        }
        /**
         * Unsubscribes all subscribed Observers.
         */
        void unsubscribeAll(){
            m_Observers.clear();
        }
        /**
         * Notifies all subscribed Observer objects by calling their update
         * function. 
         */
        void notify(){
            for(const auto& curObs : m_Observers)
                curObs.lock()->update(this->shared_from_this());
        }
    protected:
        Observable() = default;
    private:
        std::vector< std::weak_ptr< Observer<T> > > m_Observers;
    };
}
#endif //SUPPORTLIB_OBSERVER_OBSERVABLE_H