#ifndef JTS2GD_EVENT
#define JTS2GD_EVENT


// built-in
#include <sstream>
#include <list>
#include <iostream>
#include <string>
#include <cstdint>
#include <vector>

// local
#include "utils.hpp"


//
//  EventHandler
//
//
//  Buffer for events detected during code analysis.
//  
//  Instead of sending events to 'stdin' in real time, 
//  they are put on a list (for shorter insertion time) 
//  and sent to 'stdin' in the same order they were 
//  inserted during the 'flush' operation.
//
//  This class is also used to report an error detection 
//  in one of the compiler passes (eg, do not generate code if there is a syntax error).
//


enum class EventType
{
    WARNING,
    ERROR
};

const std::string EventTypeRepr[]
{
    "WARNING",
    "ERROR"
};

const Color EventTypeColor[]
{
    Color::FG_YELLOW,
    Color::FG_LIGHT_RED
};


struct Event
{
    EventType type;
    const std::string message;
    SourceLocation location;

    std::string repr() const
    {
        std::ostringstream event_repr;
        int event_idx = (int)this->type;

        event_repr << '[' 
                   << get_color(EventTypeColor[event_idx])
                   << EventTypeRepr[event_idx]
                   << get_color(Color::FG_DEFAULT)
                   << ']';
                   
        event_repr << '(' << this->location << "): ";
        
        event_repr << get_color(Color::FG_WHITE)
                   << this->message
                   << get_color(Color::FG_DEFAULT);

        return event_repr.str();
    }
};


class EventHandler
{
    private:

        std::list<Event> event_list;
        bool error = false;

    public:

        void add_error(const std::string message, SourceLocation location)
        {
            this->add_event({EventType::ERROR, std::move(message), location});
        }

        void add_warning(const std::string message, SourceLocation location)
        {
            this->add_event({EventType::WARNING, std::move(message), location});
        }

        void add_event(Event event)
        {
            if (event.type == EventType::ERROR)
                this->error = true;

            this->event_list.push_back(std::move(event));
        }

        void flush()
        {
            for (Event& event: this->event_list)
                std::cout << event.repr() << '\n';

            std::cout << std::flush;            
            this->event_list.clear();
        }

        bool has_error() const 
        {
            return this->error;
        }
};




#endif