/*
 * Category.cpp
 *
 * Copyright 2000, LifeLine Networks BV (www.lifeline.nl). All rights reserved.
 * Copyright 2000, Bastiaan Bakker. All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#include "config.h"
#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif

#include "log4cpp/OstringStream.hh"
#include "log4cpp/Category.hh"
#include "log4cpp/HierarchyMaintainer.hh"
#include "log4cpp/NDC.hh"

namespace log4cpp {

    CategoryStream::CategoryStream(Category& category, Priority::Value priority) :
        _category(category),
        _priority(priority),
        _buffer(NULL) {
    }

    CategoryStream::~CategoryStream() { 
        flush();
    }

    CategoryStream& CategoryStream::operator<<(CategoryStream::Separator separator) {
        flush();
        return *this;
    }

    void CategoryStream::flush() {
        if (_buffer) {
            getCategory().log(getPriority(), _buffer->str());
            delete _buffer;
            _buffer = NULL;
        }
    }

    Category& Category::getRoot() {
        return getInstance("");
    }

    void Category::setRootPriority(Priority::Value priority) {
        getRoot().setPriority(priority);
    }

    Priority::Value Category::getRootPriority() throw() {
        return getRoot().getPriority();
    }

    Category& Category::getInstance(const std::string& name) {
        return HierarchyMaintainer::getDefaultMaintainer().getInstance(name);
    }

    std::set<Category*>* Category::getCurrentCategories() {
        return HierarchyMaintainer::getDefaultMaintainer().
            getCurrentCategories();
    }

    void Category::shutdown() {
        HierarchyMaintainer::getDefaultMaintainer().shutdown();
    }

    Category::Category(const std::string& name, Category* parent, Priority::Value priority) : 
        _name(name),
        _parent(parent),
        _priority(priority),
        _appender(NULL),
        _ownsAppender(false),
        _isAdditive(true) {
    }

    Category::~Category() {
        if ((_appender != NULL) && ownsAppender()) {
            delete _appender;
        }
    }

    const std::string& Category::getName() const throw() {
        return _name; 
    }
    
    Priority::Value Category::getPriority() const throw() { 
        return _priority; 
    }

    void Category::setPriority(Priority::Value priority) {
        if ((priority != Priority::NOTSET) || (getParent() != NULL)) {
            _priority = priority;
        } else {
            /* caller tried to set NOTSET priority to root Category. 
               Bad caller!
               Will ignore for now.
            */
        }
    }
    
    Priority::Value Category::getChainedPriority() const throw() {
        // REQUIRE(rootCategory->getPriority() != Priority::NOTSET)
        
        const Category* c = this;
        while(c->getPriority() == Priority::NOTSET) { 
            c = c->getParent();
        }
        
        return c->getPriority();
    }
    
    void Category::setAppender(Appender* appender) {
        // REQUIRE(appender != NULL)
        if (appender == NULL)
            return;
        
        if (ownsAppender())
            delete _appender;
        
        _appender = appender;
        _ownsAppender = true;    
    }
    
    void Category::setAppender(Appender& appender) {
        if (ownsAppender())
            delete _appender;
        
        _appender = &appender;
        _ownsAppender = false;    
    }
    
    Appender* Category::getAppender() const {
        return _appender;
    }
    
    void Category::removeAllAppenders() {
         if (ownsAppender())
            delete _appender;
       
         _appender = NULL;
    }

    bool Category::ownsAppender() const throw() {
        return _ownsAppender;
    }
    
    void Category::callAppenders(const LoggingEvent& event) throw() {
        if (_appender) {
            _appender->doAppend(event);
        }

        if (getAdditivity() && (getParent() != NULL)) {
            getParent()->callAppenders(event);
        }
    }

    void Category::setAdditivity(bool additivity) {
        _isAdditive = additivity;
    }

    bool Category::getAdditivity() const throw() {
        return _isAdditive; 
    }

    Category* Category::getParent() throw() {
        return _parent; 
    }

    const Category* Category::getParent() const throw() {
        return _parent; 
    }

    void Category::_logUnconditionally(Priority::Value priority, 
                                       const char* format, 
                                       va_list arguments) throw() {
        OstringStream messageBuffer;

        messageBuffer.vform(format, arguments);
        _logUnconditionally2(priority, messageBuffer.str());
    }
    
    void Category::_logUnconditionally2(Priority::Value priority, 
                                        const std::string& message) throw() {
        LoggingEvent event(getName(), message, NDC::get(), priority);
        callAppenders(event);
    }
    
    bool Category::isPriorityEnabled(Priority::Value priority) const throw() {
        return(getChainedPriority() >= priority);
    }

    void Category::log(Priority::Value priority, 
                       const char* stringFormat, ...) throw() { 
        if (isPriorityEnabled(priority)) {
            va_list va;
            va_start(va, stringFormat);
            _logUnconditionally(priority, stringFormat, va);
            va_end(va);
        }
    }
    
    void Category::log(Priority::Value priority, 
                       const std::string& message) throw() { 
        if (isPriorityEnabled(priority))
            _logUnconditionally2(priority, message);
    }
    
    void Category::debug(const char* stringFormat, ...) throw() { 
        if (isPriorityEnabled(Priority::DEBUG)) {
            va_list va;
            va_start(va,stringFormat);
            _logUnconditionally(Priority::DEBUG, stringFormat, va);
            va_end(va);
        }
    }
    
    void Category::debug(const std::string& message) throw() { 
        if (isPriorityEnabled(Priority::DEBUG))
            _logUnconditionally2(Priority::DEBUG, message);
    }
    
    void Category::info(const char* stringFormat, ...) throw() { 
        if (isPriorityEnabled(Priority::INFO)) {
            va_list va;
            va_start(va,stringFormat);
            _logUnconditionally(Priority::INFO, stringFormat, va);
            va_end(va);
        }
    }
    
    void Category::info(const std::string& message) throw() { 
        if (isPriorityEnabled(Priority::INFO))
            _logUnconditionally2(Priority::INFO, message);
    }
    
    void Category::notice(const char* stringFormat, ...) throw() { 
        if (isPriorityEnabled(Priority::NOTICE)) {
            va_list va;
            va_start(va,stringFormat);
            _logUnconditionally(Priority::NOTICE, stringFormat, va);
            va_end(va);
        }
    }
    
    void Category::notice(const std::string& message) throw() { 
        if (isPriorityEnabled(Priority::NOTICE))
            _logUnconditionally2(Priority::NOTICE, message);
    }
    
    void Category::warn(const char* stringFormat, ...) throw() { 
        if (isPriorityEnabled(Priority::WARN)) {
            va_list va;
            va_start(va,stringFormat);
            _logUnconditionally(Priority::WARN, stringFormat, va);
            va_end(va);
        }
    }
    
    void Category::warn(const std::string& message) throw() { 
        if (isPriorityEnabled(Priority::WARN))
            _logUnconditionally2(Priority::WARN, message);
    }
    
    void Category::error(const char* stringFormat, ...) throw() { 
               if (isPriorityEnabled(Priority::ERROR)) {
            va_list va;
            va_start(va,stringFormat);
                       _logUnconditionally(Priority::ERROR, stringFormat, va);
            va_end(va);
        }
    }
    
    void Category::error(const std::string& message) throw() { 
               if (isPriorityEnabled(Priority::ERROR))
            _logUnconditionally2(Priority::ERROR, message);
    }

    void Category::crit(const char* stringFormat, ...) throw() { 
        if (isPriorityEnabled(Priority::CRIT)) {
            va_list va;
            va_start(va,stringFormat);
            _logUnconditionally(Priority::CRIT, stringFormat, va);
            va_end(va);
        }
    }
    
    void Category::crit(const std::string& message) throw() { 
        if (isPriorityEnabled(Priority::CRIT))
            _logUnconditionally2(Priority::CRIT, message);
    }

    void Category::alert(const char* stringFormat, ...) throw() { 
        if (isPriorityEnabled(Priority::ALERT)) {
            va_list va;
            va_start(va,stringFormat);
            _logUnconditionally(Priority::ALERT, stringFormat, va);
            va_end(va);
        }
    }
    
    void Category::alert(const std::string& message) throw() { 
        if (isPriorityEnabled(Priority::ALERT))
            _logUnconditionally2(Priority::ALERT, message);
    }

    void Category::emerg(const char* stringFormat, ...) throw() { 
        if (isPriorityEnabled(Priority::EMERG)) {
            va_list va;
            va_start(va,stringFormat);
            _logUnconditionally(Priority::EMERG, stringFormat, va);
            va_end(va);
        }
    }
    
    void Category::emerg(const std::string& message) throw() { 
        if (isPriorityEnabled(Priority::EMERG))
            _logUnconditionally2(Priority::EMERG, message);
    }

    CategoryStream Category::getStream(Priority::Value priority) {
        return CategoryStream(*this, isPriorityEnabled(priority) ?
                              priority : Priority::NOTSET);
    }

    CategoryStream Category::operator<<(Priority::Value priority) {
        return getStream(priority);
    }
} 

