/***************************************************************************
 * event.h
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_API_EVENT_H
#define KROSS_API_EVENT_H

#include "../main/krossconfig.h"
#include "object.h"
#include "argument.h"
#include "callable.h"
#include "list.h"
#include "exception.h"
#include "function.h"

#include <qstring.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <kdebug.h>

namespace Kross { namespace Api {

    /**
     * Template class for all kinds of callable events. An
     * event is the abstract base for callable objects like
     * methodfunctions in \a Class instances or \a EventSlot
     * and \a EventSignal to access Qt signals and slots.
     */
    template<class T>
    class Event : public Callable
    {
        private:

            /**
             * Definition of function-pointers.
             */
            typedef Object::Ptr(T::*FunctionPtr)(List::Ptr);

            /**
             * List of memberfunctions. Each function is accessible
             * by the functionname.
             */
            QMap<QString, Function* > m_functions;

        protected:

            /**
             * Add a \a Callable methodfunction to the list of functions
             * this Object supports.
             *
             * \param name The functionname. Each function this object
             *        holds should have an unique name to be
             *        still accessable.
             * \param function A pointer to the methodfunction that
             *        should handle calls.
             * \param arglist A list of arguments for the function.
             *
             * \todo Is that template arguments or concrete arguments?
             */
            void addFunction(const QString& name, FunctionPtr function, const ArgumentList& /*arglist*/ = ArgumentList())
            {
                //FIXME arglist
                m_functions.replace(name, new VarFunction0<T>(static_cast<T*>(this), function));
            }

            void addFunction(const QString& name, Function* function)
            {
                m_functions.replace(name, function);
            }

        public:

            /**
             * Constructor.
             */
            Event(const QString& name, Object::Ptr parent)
                : Callable(name, parent, ArgumentList())
            {
            }

            /**
             * Destructor.
             */
            virtual ~Event()
            {
                for(QMapIterator<QString, Function* > it = m_functions.begin(); it != m_functions.end(); ++it)
                    delete it.data();
            }

            /**
             * Overloaded method to handle function-calls.
             *
             * \throw AttributeException if argumentparameters
             *        arn't valid.
             * \throw RuntimeException if the functionname isn't
             *        valid.
             * \param name The functionname. Each function this
             *        Object holds should have a different
             *        name cause they are access by they name.
             *        If name is QString::null or empty, a
             *        self-reference to this instance is
             *        returned.
             * \param arguments The list of arguments.
             * \return An Object representing the call result
             *         or NULL if there doesn't exists such a
             *         function with defined name.
             */
            virtual Object::Ptr call(const QString& name, List::Ptr arguments)
            {
#ifdef KROSS_API_EVENT_CALL_DEBUG
                kdDebug() << QString("Event::call() name='%1' getName()='%2'").arg(name).arg(getName()) << endl;
#endif

                Function* function = m_functions[name];
                if(function) {
#ifdef KROSS_API_EVENT_CALL_DEBUG
                    kdDebug() << QString("Event::call() name='%1' is a builtin function.").arg(name) << endl;
#endif

                    //FIXME checkArguments(arguments);
                    return function->call(arguments);
                }

                if(name.isNull()) {
                    // If no name is defined, we return a reference to our instance.
                    return this;
                }

                // Redirect the call to the Kross::Api::Callable we are inheritated from.
                return Callable::call(name, arguments);
            }

    };

}}

#endif

